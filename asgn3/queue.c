#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#include "queue.h"

// Define the queue structure
struct queue {
    int size; // Maximum size of the queue
    int front; // Index for the front of the queue
    int end; // Index for the end of the queue
    void **data; // Data array to store elements
    pthread_mutex_t mutex; // Mutex for thread safety
    pthread_cond_t not_empty; // Condition variable for non-empty queue
    pthread_cond_t not_full; // Condition variable for non-full queue
};

// Create a new queue with the specified size
queue_t *queue_new(int size) {
    queue_t *q = (queue_t *) malloc(sizeof(queue_t)); // Allocate memory for the queue
    if (q == NULL) {
        return NULL;
    }

    q->size = size; // Set the maximum size of the queue
    q->front = 0; // Initialize the front index
    q->end = 0; // Initialize the last index

    // Allocate memory for the data array to store elements
    q->data = (void **) malloc(sizeof(void *) * size);
    if (q->data == NULL) {
        free(q);
        return NULL;
    }

    // Initialize a mutex for thread safety
    pthread_mutex_init(&q->mutex, NULL);

    // Initialize condition variables for non-empty and non-full queues
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);

    return q; // Return the initialized queue
}

// Delete the queue and free allocated memory
void queue_delete(queue_t **q) {
    if (q == NULL || *q == NULL) {
        return;
    }

    queue_t *queue = *q; // Get the pointer to the queue
    free(queue->data); // Free the memory used by the data array

    // Destroy the mutex and condition variables
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);

    free(queue); // Free the memory used by the queue
    *q = NULL; // Set the pointer to the queue to NULL
}

// Push an element onto the queue
bool queue_push(queue_t *q, void *elem) {
    pthread_mutex_lock(&q->mutex); // Lock the mutex for thread safety

    // Wait until the queue is not full
    while ((q->end + 1) % q->size == q->front) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    // Store the element in the data array and update the last index
    q->data[q->end] = elem;
    q->end = (q->end + 1) % q->size;

    // Signal that the queue is not empty and unlock the mutex
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);

    return true; // Return true to indicate a successful push
}

// Pop an element from the queue
bool queue_pop(queue_t *q, void **elem) {
    pthread_mutex_lock(&q->mutex); // Lock the mutex for thread safety

    // Wait until the queue is not empty
    while (q->front == q->end) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    // Retrieve the element from the data array and update the front index
    *elem = q->data[q->front];
    q->front = (q->front + 1) % q->size;

    // Signal that the queue is not full and unlock the mutex
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);

    return true; // Return true to indicate a successful pop
}
