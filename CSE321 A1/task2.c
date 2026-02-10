#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// ---------------- Global Variables ----------------
pthread_mutex_t table_mutex;
sem_t supplier_sem, makerA_sem, makerB_sem, makerC_sem;

int ingredient1, ingredient2;
int N;          // number of times supplier places ingredients
int done = 0;   // flag to tell makers supplier is done

// ---------------- Helper Function ----------------
void place_ingredients()
{
    int options[3][2] = {{1,2}, {1,3}, {2,3}}; // 1-Bread, 2-Cheese, 3-Lettuce
    int choice = rand() % 3;

    ingredient1 = options[choice][0];
    ingredient2 = options[choice][1];

    printf("Supplier places: ");
    if (ingredient1 == 1) printf("Bread ");
    if (ingredient1 == 2) printf("Cheese ");
    if (ingredient1 == 3) printf("Lettuce ");

    printf("and ");

    if (ingredient2 == 1) printf("Bread\n");
    if (ingredient2 == 2) printf("Cheese\n");
    if (ingredient2 == 3) printf("Lettuce\n");

    // Signal the right maker
    if ((ingredient1 == 2 && ingredient2 == 3) || (ingredient1 == 3 && ingredient2 == 2))
        sem_post(&makerA_sem); // Maker A has Bread
    else if ((ingredient1 == 1 && ingredient2 == 3) || (ingredient1 == 3 && ingredient2 == 1))
        sem_post(&makerB_sem); // Maker B has Cheese
    else
        sem_post(&makerC_sem); // Maker C has Lettuce
}

// ---------------- Sandwich Maker Functions ----------------
void* makerA(void* arg)
{
    while (1)
    {
        sem_wait(&makerA_sem);
        if (done) break;

        pthread_mutex_lock(&table_mutex);
        printf("Maker A picks up Cheese and Lettuce\n");
        printf("Maker A is making the sandwich...\n");
        sleep(1);
        printf("Maker A finished making the sandwich and eats it\n");
        pthread_mutex_unlock(&table_mutex);

        printf("Maker A signals Supplier\n");
        sem_post(&supplier_sem);
    }
    return NULL;
}

void* makerB(void* arg)
{
    while (1)
    {
        sem_wait(&makerB_sem);
        if (done) break;

        pthread_mutex_lock(&table_mutex);
        printf("Maker B picks up Bread and Lettuce\n");
        printf("Maker B is making the sandwich...\n");
        sleep(1);
        printf("Maker B finished making the sandwich and eats it\n");
        pthread_mutex_unlock(&table_mutex);

        printf("Maker B signals Supplier\n");
        sem_post(&supplier_sem);
    }
    return NULL;
}

void* makerC(void* arg)
{
    while (1)
    {
        sem_wait(&makerC_sem);
        if (done) break;

        pthread_mutex_lock(&table_mutex);
        printf("Maker C picks up Bread and Cheese\n");
        printf("Maker C is making the sandwich...\n");
        sleep(1);
        printf("Maker C finished making the sandwich and eats it\n");
        pthread_mutex_unlock(&table_mutex);

        printf("Maker C signals Supplier\n");
        sem_post(&supplier_sem);
    }
    return NULL;
}

// ---------------- Supplier Function ----------------
void* supplier(void* arg)
{
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_lock(&table_mutex);
        place_ingredients();
        pthread_mutex_unlock(&table_mutex);

        sleep(1); // optional small delay
    }

    done = 1; // tell makers to exit
    sem_post(&makerA_sem);
    sem_post(&makerB_sem);
    sem_post(&makerC_sem);

    return NULL;
}

// ---------------- Main ----------------
int main()
{
    srand(time(NULL));
    printf("Enter number of times supplier places ingredients:\n");
    scanf("%d", &N);

    // Initialize mutex and semaphores
    pthread_mutex_init(&table_mutex, NULL);
    sem_init(&supplier_sem, 0, 1); // table initially free
    sem_init(&makerA_sem, 0, 0);
    sem_init(&makerB_sem, 0, 0);
    sem_init(&makerC_sem, 0, 0);

    pthread_t sup, A, B, C;

    // Create threads
    pthread_create(&sup, NULL, supplier, NULL);
    pthread_create(&A, NULL, makerA, NULL);
    pthread_create(&B, NULL, makerB, NULL);
    pthread_create(&C, NULL, makerC, NULL);

    // Join threads
    pthread_join(sup, NULL);
    pthread_join(A, NULL);
    pthread_join(B, NULL);
    pthread_join(C, NULL);

    // Destroy mutex and semaphores
    pthread_mutex_destroy(&table_mutex);
    sem_destroy(&supplier_sem);
    sem_destroy(&makerA_sem);
    sem_destroy(&makerB_sem);
    sem_destroy(&makerC_sem);

    return 0;
}
