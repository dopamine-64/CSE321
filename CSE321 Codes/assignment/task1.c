#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int n;  // fibonacci limit
int s;  // number of searches
int *search_indices;  // indices to search
int *fib; // fibonacci array

// ----------------------- Thread 1 -----------------------
void* generate_fib(void *arg)
{
    fib = (int*) malloc((n + 1) * sizeof(int));

    if (n >= 0) fib[0] = 0;
    if (n >= 1) fib[1] = 1;

    for (int i = 2; i <= n; i++)
        fib[i] = fib[i - 1] + fib[i - 2];

    pthread_exit((void*)fib);
}

// ----------------------- Thread 2 -----------------------
void* search_fib(void *arg)
{
    int *results = (int*) malloc(s * sizeof(int));

    for (int i = 0; i < s; i++)
    {
        if (search_indices[i] >= 0 && search_indices[i] <= n)
            results[i] = fib[search_indices[i]];
        else
            results[i] = -1;
    }

    pthread_exit((void*)results);
}

// ----------------------- Main -----------------------
int main()
{
    pthread_t t1, t2;

    printf("Enter the term of fibonacci sequence:\n");
    scanf("%d", &n);
    while (n < 0 || n > 40) {
	printf("Invalid! Enter n between 0 and 40:\n");
	scanf("%d", &n);
    }

    printf("How many numbers you are willing to search?:\n");
    scanf("%d", &s);
    while (s <= 0) {
    	printf("Invalid! Number of searches must be greater than 0:\n");
    	scanf("%d", &s);
    }

    search_indices = (int*) malloc(s * sizeof(int));

    for (int i = 0; i < s; i++)
	{
	    printf("Enter search %d:\n", i + 1);

	    while (scanf("%d", &search_indices[i]) != 1) {
		printf("Invalid input! Enter an integer:\n");
		while (getchar() != '\n'); // clear buffer
	    }
	}

    // Thread 1: Generate Fibonacci
    pthread_create(&t1, NULL, generate_fib, NULL);
    pthread_join(t1, (void**)&fib);

    // Print Fibonacci sequence
    for (int i = 0; i <= n; i++)
        printf("a[%d] = %d\n", i, fib[i]);

    // Thread 2: Search Fibonacci values
    int *results;
    pthread_create(&t2, NULL, search_fib, NULL);
    pthread_join(t2, (void**)&results);

    // Print results
    for (int i = 0; i < s; i++)
        printf("result of search #%d = %d\n", i + 1, results[i]);

    // Free memory
    free(fib);
    free(search_indices);
    free(results);

    return 0;
}
