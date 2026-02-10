#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid1, pid2;

    pid1 = fork();  // Create child

    if (pid1 < 0) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) { 
        // ----- CHILD PROCESS -----
        pid2 = fork();  // Create grandchild from child

        if (pid2 < 0) {
            perror("fork");
            return 1;
        }

        if (pid2 == 0) {
            // ----- GRANDCHILD -----
            printf("I am grandchild\n");
        } else {
            // CHILD waits so that grandchild prints first
            wait(NULL);
            printf("I am child\n");
        }

    } else {
        // PARENT waits so child prints after grandchild
        wait(NULL);
        printf("I am parent\n");
    }

    return 0;
}
