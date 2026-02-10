#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int n;

    printf("Enter a number (-10 to 10): ");
    scanf("%d", &n);

    printf("Captain Nova PID = %d\n", getpid());
    printf("Captain Nova: Launching co-pilot Orion...\n");

    pid_t child = fork();

    if (child < 0) {
        perror("fork failed");
        exit(1);
    }

    if (child == 0) {
        // ====================== ORION (CHILD) ======================
        printf("Co-pilot Orion (Child) PID=%d, PPID=%d Orion executing\n",
               getpid(), getppid());

        printf("Mission Code Check\n");

        // Replace with your student ID
        char *studentID = "48291037";

        for (int i = 0; i < 5; i++) {
            printf("%s\n", studentID);
        }

        // -------- ORION creates Navigation Bot (grandchild) --------
        pid_t grandchild = fork();

        if (grandchild < 0) {
            perror("fork failed");
            exit(1);
        }

        if (grandchild == 0) {
            // ================== NAVIGATION BOT (GRANDCHILD) ==================
            printf("Navigation Bot (Grandchild): PID=%d, PPID=%d Navigation Bot executing ",
                   getpid(), getppid());

            if (n > 0)
                printf("%d is positive.\n", n);
            else if (n < 0)
                printf("%d is negative.\n", n);
            else
                printf("%d is zero.\n", n);

            exit(0);
        }

        // Child waits for Navigation Bot
        wait(NULL);
        exit(0);
    }

    // ====================== CAPTAIN NOVA (PARENT) ======================
    wait(NULL);  // wait for child
    wait(NULL);  // extra wait for grandchild

    printf("Captain Nova: All missions completed.\n");

    return 0;
}
