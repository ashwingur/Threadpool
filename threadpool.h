#ifndef POOL
#define POOL

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// Used internally, represents a node in the queue
struct task{
    void *(*function)(void *);
    void *args;
    struct task *next;
};

// Threadpool object that contains all the data for the functions
struct threadpool {
    pthread_mutex_t lock; // Lock used when changing the other variables in this struct
    struct task *head;    // Pointer to the front of the queue to pop tasks
    struct task *tail;    // Pointer to the end of the queue to append tasks
    int queue_size;       // Current size of the queue
    int n_threads;        // Number of threads the threadpool contains
    pthread_t *threads;   // Thread object array
    pthread_cond_t cond;  // Condition variable for sending singals to the threads
    int stop;             // Flag to indicate if the threadpool is shutting down
};

// Initialises the threadpool with the given number of threads to be used
struct threadpool* threadpool_create(int n_threads);

// Waits for current and pending tasks to be completed, terminates threads, and cleans up
void threadpool_destroy(struct threadpool* threadpool);

// Used by the user to add a task to the queue, giving a function and the arguments to be used in the function
// Returns 0 if successful, -1 if unsuccessful
int enqueue_task(void*(*function)(void*), void * args, struct threadpool* threadpool);

// Used within run_thread by the threads to get a task to execute
struct task *dequeue_task(struct threadpool *threadpool);

// The threads within the pool live within this function
void* run_thread(void *args);
#endif