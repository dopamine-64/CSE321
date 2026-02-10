#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t child, gc1, gc2, gc3;

    // ----------- PARENT CREATES ONE CHILD -----------
    child = fork();

    if (child < 0) {
        perror("fork");
        return 1;
    }

    if (child == 0) {
        // ================= CHILD PROCESS =================
        printf("Child process ID: %d (Parent: %d)\n", getpid(), getppid());

        // ----------- CHILD CREATES 3 GRANDCHILDREN -----------

        gc1 = fork();
        if (gc1 == 0) {
            printf("Grand Child process ID: %d (Parent: %d)\n", getpid(), getppid());
            return 0;
        }

        gc2 = fork();
        if (gc2 == 0) {
            printf("Grand Child process ID: %d (Parent: %d)\n", getpid(), getppid());
            return 0;
        }

        gc3 = fork();
        if (gc3 == 0) {
            printf("Grand Child process ID: %d (Parent: %d)\n", getpid(), getppid());
            return 0;
        }

        // Child waits for the 3 grandchildren
        wait(NULL);
        wait(NULL);
        wait(NULL);

    } else {
        // ================= PARENT PROCESS =================
        printf("Parent process ID: %d\n", getpid());

        // Parent waits for the child
        wait(NULL);
    }

    return 0;
}
