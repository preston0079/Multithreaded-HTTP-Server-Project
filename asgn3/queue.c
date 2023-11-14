#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#include "queue.h"

// Work from previous class was referenced
// Help recieved from classmates

struct queue {
    int size; // Maximum size of the queue
    int front; // Index for the front of the queue
    int end; // Index for the end of the queue
    void **data; // Data array

    pthread_mutex_t mutex; // Mutex
    pthread_cond_t not_empty; // Condition variable for not-empty queue
    pthread_cond_t not_full; // Condition variable for not-full queue
};

queue_t *queue_new(int size) {

    // Allocate memory for the queue
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    // Assert
    if (q == NULL) {
        return NULL;
    }

    q->size = size; // Set the maximum size of the queue with (int size)
    q->front = 0; // Initialize front index
    q->end = 0; // Initialize last index

    // Allocate memory for the data
    q->data = (void **) malloc(sizeof(void *) * size);
    // Assert
    if (q->data == NULL) {
        free(q);
        return NULL;
    }

    // Initialize mutex
    pthread_mutex_init(&q->mutex, NULL);

    // Initialize condition variables for non-empty and non-full queues
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);

    return q;
}

void queue_delete(queue_t **q) {

    if (q != NULL || *q != NULL) {
        queue_t *queue = *q;
        free(queue->data); // Free data array

        // Destroy the mutex and condition variables
        pthread_mutex_destroy(&queue->mutex);
        pthread_cond_destroy(&queue->not_empty);
        pthread_cond_destroy(&queue->not_full);

        free(queue); // Free queue
        *q = NULL; // Set pointer to NULL
    }
}

bool queue_push(queue_t *q, void *elem) {

    pthread_mutex_lock(&q->mutex); // Lock the mutex

    // Wait until the queue is not full
    while ((q->end + 1) % q->size == q->front) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    // Followed structure from lectures
    q->data[q->end] = elem;
    q->end = (q->end + 1) % q->size;
    // q->size++;

    // Signal that the queue is not empty and unlock the mutex
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);

    return true;
}

bool queue_pop(queue_t *q, void **elem) {

    pthread_mutex_lock(&q->mutex); // Lock the mutex

    // Wait until the queue is not empty
    while (q->front == q->end) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    // Followed structure from lectures
    *elem = q->data[q->front];
    q->front = (q->front + 1) % q->size;
    // q->size--;

    // Signal that the queue is not full and unlock the mutex
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);

    return true;
}
