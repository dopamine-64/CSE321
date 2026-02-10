#include <stdio.h>
#include <pthread.h>

int numbers[4];               // shared array to store thread inputs
pthread_mutex_t lock;        // to protect index access
int index_pos = 0;           // next position to fill

void* read_number(void* arg) {
    int thread_id = *(int*)arg;
    int value;

    printf("Thread %d, enter a number: ", thread_id);
    scanf("%d", &value);

    // critical section
    pthread_mutex_lock(&lock);
    numbers[index_pos] = value;
    index_pos++;
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main() {
    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4};

    pthread_mutex_init(&lock, NULL);

    // create 4 threads
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, read_number, &thread_ids[i]);
    }

    // wait for all threads to finish
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    // print numbers and calculate product
    int product = 1;
    for (int i = 0; i < 4; i++) {
        printf("Thread %d input: %d\n", i+1, numbers[i]);
        product *= numbers[i];
    }

    printf("Product of all numbers: %d\n", product);

    pthread_mutex_destroy(&lock);

    return 0;
}
