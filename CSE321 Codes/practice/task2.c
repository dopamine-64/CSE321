#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // ------------------ CHILD PROCESS ------------------
        wait(NULL); // ensure parent is done writing

        printf("Child process reading from file...\n");

        FILE *fp = fopen("output.txt", "r");
        if (!fp) {
            perror("File open failed");
            exit(1);
        }

        char buffer[200];
        fgets(buffer, sizeof(buffer), fp);
        fclose(fp);

        printf("Content read: %s\n", buffer);
    } 
    else {
        // ------------------ PARENT PROCESS ------------------
        printf("Parent process writing to file...\n");

        FILE *fp = fopen("output.txt", "w");
        if (!fp) {
            perror("File open failed");
            exit(1);
        }

        fprintf(fp, "Hello from Parent Process");
        fclose(fp);

        // Parent waits so child starts after writing
        wait(NULL);
    }

    return 0;
}
