#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>

char sentence[200];  // shared array

void* threadA(void* arg) {
    printf("Enter a sentence: ");
    fgets(sentence, sizeof(sentence), stdin);
    sentence[strcspn(sentence, "\n")] = '\0';  // remove newline

    printf("Thread A: Received sentence \"%s\"\n", sentence);
    return NULL;
}

void* threadB(void* arg) {
    printf("Thread B: Analyzing sentence...\n");

    int upper = 0, lower = 0;
    for (int i = 0; sentence[i] != '\0'; i++) {
        if (isupper(sentence[i])) upper++;
        if (islower(sentence[i])) lower++;
    }

    printf("Uppercase letters = %d\n", upper);
    printf("Lowercase letters = %d\n", lower);

    return NULL;
}

int main() {
    pthread_t tA, tB;

    // Create Thread A
    pthread_create(&tA, NULL, threadA, NULL);
    pthread_join(tA, NULL);  // wait for input first

    // Create Thread B
    pthread_create(&tB, NULL, threadB, NULL);
    pthread_join(tB, NULL);  // wait for analysis

    printf("All threads completed.\n");

    return 0;
}
