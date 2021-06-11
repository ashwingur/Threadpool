#include "threadpool.h"

struct threadpool* threadpool_create(int n_threads){
    // Initialise all fields
    struct threadpool *pool = malloc(sizeof(struct threadpool));
    pool->n_threads = n_threads;
    pool->queue_size = 0;
    pool->threads = malloc(sizeof(pthread_t) * n_threads);
    pool->stop = 0;
    pthread_cond_init(&pool->cond, NULL);
    pthread_mutex_init(&pool->lock, NULL);
    for (int i = 0; i < n_threads; i++){
        // Create the threads
        pthread_create(&pool->threads[i], NULL, run_thread, pool);
    }
    return pool;
}

int enqueue_task(void*(*function)(void*), void *args, struct threadpool* threadpool){
    if (threadpool == NULL || function == NULL){
        // Invalid threadpool or function;
        return -1;
    }
    pthread_mutex_lock(&threadpool->lock);
    if (threadpool->stop){
        pthread_mutex_unlock(&threadpool->lock);
        return -1;
    }
    struct task *new_task = malloc(sizeof(struct task));
    new_task->function = function;
    new_task->args = args;
    new_task->next = NULL;

    // Push new task
    if (threadpool->queue_size == 0){
        // Queue is empty so this task also becomes the head;
        threadpool->head = new_task;
    } else {
        // Otherwise append it to the current tail
        threadpool->tail->next = new_task;
    }
    threadpool->tail = new_task;
    threadpool->queue_size++;
    pthread_mutex_unlock(&threadpool->lock);
    // Signal to a thread that there is a task available
    pthread_cond_signal(&threadpool->cond);
    return 0;
}

struct task *dequeue_task(struct threadpool *threadpool){
    // No need for mutex here because dequeue it being called in a race free location
    struct task *task;
    if (threadpool->queue_size > 0){
        // There is a task available
        task = threadpool->head;
        threadpool->head = task->next;
        threadpool->queue_size--;
    } else {
        task = NULL;
    }
    return task;
}

void *run_thread(void *args){
    struct threadpool* threadpool = args;
    while (1){
        struct task *task;
        pthread_mutex_lock(&threadpool->lock);
        while (threadpool->queue_size == 0 && !threadpool->stop){
            // There are currently no tasks to be processed and the threadpool has not yet
            // been terminated, so wait here.
            pthread_cond_wait(&threadpool->cond, &threadpool->lock);
        }

        if (threadpool->stop && threadpool->queue_size == 0){
                // No tasks left and the threadpool is being shutdown
                pthread_mutex_unlock(&threadpool->lock);
                pthread_exit(NULL);
        }

        // Try get a task from the queue
        task = dequeue_task(threadpool);
        pthread_mutex_unlock(&threadpool->lock);
        if (task != NULL){
            task->function(task->args);
            free(task);
        }
    }
}

void threadpool_destroy(struct threadpool* threadpool){
    pthread_mutex_lock(&threadpool->lock);
    // Set flag
    threadpool->stop = 1;
    pthread_mutex_unlock(&threadpool->lock);
    // Broadcast all waiting threads to stop blocking
    pthread_cond_broadcast(&threadpool->cond);
    
    // Wait for every thread to finish
    for (int i = 0; i < threadpool->n_threads; i++){
        pthread_join(threadpool->threads[i], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&threadpool->lock);
    pthread_cond_destroy(&threadpool->cond);
    free(threadpool->threads);
    free(threadpool);
}