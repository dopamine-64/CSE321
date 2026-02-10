#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    int shm_id;
    int *process_count;

    // Create shared memory (one integer)
    shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }

    process_count = (int *) shmat(shm_id, NULL, 0);
    if (process_count == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    // Initialize count = 1 (the original parent)
    *process_count = 1;

    // Every time a new process is created, increment count
    pid_t a, b, c;

    // First fork
    a = fork();
    if (a == 0 || a > 0) (*process_count)++;

    // Second fork
    b = fork();
    if (b == 0 || b > 0) (*process_count)++;

    // Third fork
    c = fork();
    if (c == 0 || c > 0) (*process_count)++;

    // Now check if PID is odd → create one more child
    if (getpid() % 2 == 1) {
        pid_t extra = fork();
        if (extra == 0 || extra > 0) (*process_count)++;
    }

    // Let parent wait so final count prints last
    wait(NULL);
    wait(NULL);
    wait(NULL);

    // Only the original parent should print final count
    if (getppid() != 1 && getpid() == getpid()) {
        // Try to detect the original parent:
        if (getppid() == 1) {
            // ignore init-adopted processes
        }
    }

    // Original parent will be the only one whose parent was the shell
    if (getppid() != 1 && a != 0 && b != 0 && c != 0) {
        printf("Total number of processes created: %d\n", *process_count);
    }

    shmdt(process_count);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
