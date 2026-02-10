#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "a"); // "a" creates file if it doesn't exist
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    char input[200];

    while (1) {
        printf("Enter a string (enter -1 to stop): ");
        fgets(input, sizeof(input), stdin);

        // Remove trailing newline
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "-1") == 0) {
            break;
        }

        fprintf(fp, "%s\n", input);
    }

    fclose(fp);
    printf("Data written successfully.\n");

    return 0;
}
