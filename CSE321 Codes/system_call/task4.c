// fork_sort_oddeven.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int compare(const void *a, const void *b) {
    return (*(int*)b - *(int*)a); // descending
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s num1 num2 num3 ...\n", argv[0]);
        return 1;
    }

    int n = argc - 1;
    int arr[n];

    for (int i = 1; i < argc; i++)
        arr[i - 1] = atoi(argv[i]);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // -------- CHILD: SORT THE ARRAY --------
        qsort(arr, n, sizeof(int), compare);

        printf("Child: Sorted array (descending): ");
        for (int i = 0; i < n; i++)
            printf("%d ", arr[i]);
        printf("\n");

        // Child exits after sorting and printing
        exit(0);
    } 
    else {
        // -------- PARENT: WAIT & THEN PRINT ODD/EVEN --------
        wait(NULL);

        printf("Parent: Odd/Even Status:\n");
        for (int i = 0; i < n; i++) {
            if (arr[i] % 2 == 0)
                printf("%d is even\n", arr[i]);
            else
                printf("%d is odd\n", arr[i]);
        }
    }

    return 0;
}
