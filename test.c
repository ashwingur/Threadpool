#include "threadpool.h"

#define SIZE 100000000
#define NTASKS 10
#define NPROCESSORS 4

void *worker(void *arg){
    // Simple test function that increments a number SIZE times and prints the result
    int sum = 0;
    for (int i = 0; i < SIZE; i++){
        sum++;
    }
    printf("%d\n", sum);
    return NULL;
}

int main(){
    struct threadpool *pool = threadpool_create(NPROCESSORS);
    for (int i = 0; i < NTASKS; i++){
        enqueue_task(worker, NULL, pool);
    }

    threadpool_destroy(pool);
    return 0;
}