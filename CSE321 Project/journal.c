#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#define FS_MAGIC 0x56534653U
#define JOURNAL_MAGIC 0x4A524E4CU 

#define BLOCK_SIZE        4096U
#define JOURNAL_BLOCK_IDX    1U
#define JOURNAL_BLOCKS      16U
#define INODE_BMAP_IDX     (JOURNAL_BLOCK_IDX + JOURNAL_BLOCKS)
#define DATA_BMAP_IDX      (INODE_BMAP_IDX + 1U)
#define INODE_START_IDX    (DATA_BMAP_IDX + 1U)
#define DATA_START_IDX     (INODE_START_IDX + 2U)

#define DEFAULT_IMAGE "vsfs.img"




struct journal_header {
    uint32_t magic;
    uint32_t nbytes_used;
};

struct rec_header {
    uint16_t type;
    uint16_t size;
};

struct inode {
    uint16_t type;
    uint16_t links;
    uint32_t size;
    uint32_t direct[8];
    uint32_t ctime;
    uint32_t mtime;
    uint8_t  _pad[128 - (2 + 2 + 4 + 8*4 + 4 + 4)];
};

struct dirent {
    uint32_t inode;
    char name[28];
};



#define REC_DATA   1
#define REC_COMMIT 2

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void read_block(int fd, uint32_t block_no, void *buf) {
    if (pread(fd, buf, BLOCK_SIZE, (off_t)block_no * BLOCK_SIZE) != BLOCK_SIZE) {
        die("pread");
    }
}

static void write_block(int fd, uint32_t block_no, const void *buf) {
    if (pwrite(fd, buf, BLOCK_SIZE, (off_t)block_no * BLOCK_SIZE) != BLOCK_SIZE) {
        die("pwrite");
    }
}

static struct journal_header *get_journal_header(uint8_t *journal_block) {
    return (struct journal_header *)journal_block;
}
static void journal_append_data(int fd, uint32_t block_no, const void *data) {
    uint8_t journal[BLOCK_SIZE];
    read_block(fd, JOURNAL_BLOCK_IDX, journal);

    struct journal_header *jh = get_journal_header(journal);

    uint32_t offset = jh->nbytes_used;

    struct rec_header rh;
    rh.type = REC_DATA;
    rh.size = sizeof(struct rec_header) + sizeof(uint32_t) + BLOCK_SIZE;

    if (offset + rh.size > BLOCK_SIZE * JOURNAL_BLOCKS) {
        fprintf(stderr, "Journal full, run install\n");
        exit(1);
    }

    pwrite(fd, &rh, sizeof(rh),
           (off_t)JOURNAL_BLOCK_IDX * BLOCK_SIZE + offset);
    offset += sizeof(rh);

    pwrite(fd, &block_no, sizeof(block_no),
           (off_t)JOURNAL_BLOCK_IDX * BLOCK_SIZE + offset);
    offset += sizeof(block_no);

    pwrite(fd, data, BLOCK_SIZE,
           (off_t)JOURNAL_BLOCK_IDX * BLOCK_SIZE + offset);
    offset += BLOCK_SIZE;

    jh->nbytes_used = offset;
    write_block(fd, JOURNAL_BLOCK_IDX, journal);
}
static void journal_append_commit(int fd) {
    uint8_t journal[BLOCK_SIZE];
    read_block(fd, JOURNAL_BLOCK_IDX, journal);

    struct journal_header *jh = get_journal_header(journal);
    uint32_t offset = jh->nbytes_used;

    struct rec_header rh;
    rh.type = REC_COMMIT;
    rh.size = sizeof(struct rec_header);

    if (offset + rh.size > BLOCK_SIZE * JOURNAL_BLOCKS) {
        fprintf(stderr, "Journal full, run install\n");
        exit(1);
    }

    pwrite(fd, &rh, sizeof(rh),
           (off_t)JOURNAL_BLOCK_IDX * BLOCK_SIZE + offset);
    offset += sizeof(rh);

    jh->nbytes_used = offset;
    write_block(fd, JOURNAL_BLOCK_IDX, journal);
}


static void init_journal(int fd) {
    uint8_t block[BLOCK_SIZE];
    struct journal_header *jh = (struct journal_header *)block;

    read_block(fd, JOURNAL_BLOCK_IDX, block);

    if (jh->magic != JOURNAL_MAGIC) {

        memset(block, 0, BLOCK_SIZE);
        jh->magic = JOURNAL_MAGIC;
        jh->nbytes_used = sizeof(struct journal_header);
        write_block(fd, JOURNAL_BLOCK_IDX, block);
    }
}
static int bitmap_test(uint8_t *bm, uint32_t idx) {
    return (bm[idx / 8] >> (idx % 8)) & 1;
}

static void bitmap_set(uint8_t *bm, uint32_t idx) {
    bm[idx / 8] |= (1 << (idx % 8));
}

static void do_create(int fd, const char *name) {
    uint8_t inode_bm[BLOCK_SIZE];
    uint8_t inode_blocks[2 * BLOCK_SIZE];
    uint8_t dir_block[BLOCK_SIZE];

    read_block(fd, INODE_BMAP_IDX, inode_bm);
    read_block(fd, INODE_START_IDX, inode_blocks);
    read_block(fd, INODE_START_IDX + 1, inode_blocks + BLOCK_SIZE);
    read_block(fd, DATA_START_IDX, dir_block);

    int new_ino = -1;
    int total_inodes = (2 * BLOCK_SIZE) / sizeof(struct inode);

    for (int i = 0; i < total_inodes; i++) {
        if (!bitmap_test(inode_bm, i)) {
            new_ino = i;
            break;
        }
    }

    if (new_ino < 0) {
        fprintf(stderr, "No free inode available\n");
        exit(1);
    }

    bitmap_set(inode_bm, new_ino);

    struct inode *inodes = (struct inode *)inode_blocks;
    struct inode *ino = &inodes[new_ino];

    memset(ino, 0, sizeof(*ino));
    ino->type = 1;
    ino->links = 1;
    ino->size = 0;

    struct dirent *ents = (struct dirent *)dir_block;
    int max_entries = BLOCK_SIZE / sizeof(struct dirent);
    int slot = -1;

    for (int i = 0; i < max_entries; i++) {
        if (ents[i].inode == 0) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        fprintf(stderr, "No free directory entry\n");
        exit(1);
    }

    ents[slot].inode = new_ino;
    strncpy(ents[slot].name, name, sizeof(ents[slot].name) - 1);
    ents[slot].name[sizeof(ents[slot].name) - 1] = '\0';

    journal_append_data(fd, INODE_BMAP_IDX, inode_bm);

    uint32_t inode_block_no =
        INODE_START_IDX + (new_ino * sizeof(struct inode)) / BLOCK_SIZE;

    if (inode_block_no == INODE_START_IDX) {
        journal_append_data(fd, INODE_START_IDX, inode_blocks);
    } else {
        journal_append_data(fd, INODE_START_IDX + 1,
                            inode_blocks + BLOCK_SIZE);
    }

    journal_append_data(fd, DATA_START_IDX, dir_block);

    journal_append_commit(fd);
    printf("Logged '%s' in journal\n", name);

}

static void do_install(int fd) {
    uint8_t journal_buf[BLOCK_SIZE * JOURNAL_BLOCKS];
    off_t journal_offset = (off_t)JOURNAL_BLOCK_IDX * BLOCK_SIZE;

    if (pread(fd, journal_buf, sizeof(journal_buf), journal_offset) != sizeof(journal_buf)) {
        die("pread journal");
    }

    struct journal_header *jh = (struct journal_header *)journal_buf;

    if (jh->magic != JOURNAL_MAGIC) {
        fprintf(stderr, "No valid journal to install\n");
        return;
    }

    uint32_t pos = sizeof(struct journal_header);
    uint32_t limit = jh->nbytes_used;

    struct {
        uint32_t block_no;
        uint8_t  data[BLOCK_SIZE];
    } txn_data[32];

    int txn_count = 0;

    while (pos + sizeof(struct rec_header) <= limit) {
        struct rec_header *rh = (struct rec_header *)(journal_buf + pos);

        if (pos + rh->size > limit) {
            break;
        }

        if (rh->type == REC_DATA) {
            uint32_t *block_no_ptr =
                (uint32_t *)(journal_buf + pos + sizeof(struct rec_header));
            uint8_t *data_ptr =
                journal_buf + pos + sizeof(struct rec_header) + sizeof(uint32_t);

            txn_data[txn_count].block_no = *block_no_ptr;
            memcpy(txn_data[txn_count].data, data_ptr, BLOCK_SIZE);
            txn_count++;
        }
        else if (rh->type == REC_COMMIT) {

            for (int i = 0; i < txn_count; i++) {
                write_block(fd, txn_data[i].block_no, txn_data[i].data);
            }
            printf("Installed committed transactions in journal\n\n");
            txn_count = 0;
        }
        else {
        
            break;
        }

        pos += rh->size;
    }

    uint8_t clear_block[BLOCK_SIZE];
    memset(clear_block, 0, BLOCK_SIZE);

    struct journal_header *new_jh = (struct journal_header *)clear_block;
    new_jh->magic = JOURNAL_MAGIC;
    new_jh->nbytes_used = sizeof(struct journal_header);

    write_block(fd, JOURNAL_BLOCK_IDX, clear_block);
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <create|install> [name]\n", argv[0]);
        return 1;
    }

    int fd = open(DEFAULT_IMAGE, O_RDWR);
    if (fd < 0) {
        die("open");
    }

    init_journal(fd);

    if (strcmp(argv[1], "create") == 0) {
        if (argc < 3) {
            fprintf(stderr, "create requires a name\n");
            close(fd);
            return 1;
        }
        do_create(fd, argv[2]);
    }
    else if (strcmp(argv[1], "install") == 0) {
        do_install(fd);
    }

    close(fd);
    return 0;
}

