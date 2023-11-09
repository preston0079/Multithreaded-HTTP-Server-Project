#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#include "rwlock.h"

// Define the structure for the reader-writer lock
typedef struct rwlock {
    int readers; // Number of current readers
    int writers; // Number of current writers
    int n; // N-WAY parameter
    PRIORITY priority; // Reader-writer lock priority
    pthread_mutex_t mutex; // Mutex for thread safety
    pthread_cond_t readers_cond; // Condition variable for readers
    pthread_cond_t writers_cond; // Condition variable for writers
} rwlock_t;

// Create a new reader-writer lock
rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *rwlock
        = (rwlock_t *) malloc(sizeof(rwlock_t)); // Allocate memory for the reader-writer lock
    if (rwlock == NULL) {
        return NULL;
    }

    rwlock->readers = 0; // Initialize the number of readers to 0
    rwlock->writers = 0; // Initialize the number of writers to 0
    rwlock->n = n; // Set the N-WAY parameter
    rwlock->priority = p; // Set the reader-writer lock priority

    // Initialize the mutex and condition variables for readers and writers
    pthread_mutex_init(&rwlock->mutex, NULL);
    pthread_cond_init(&rwlock->readers_cond, NULL);
    pthread_cond_init(&rwlock->writers_cond, NULL);

    return rwlock; // Return the initialized reader-writer lock
}

// Delete the reader-writer lock and free allocated memory
void rwlock_delete(rwlock_t **l) {
    if (l == NULL || *l == NULL) {
        return;
    }

    rwlock_t *rwlock = *l;

    // Destroy the mutex and condition variables
    pthread_mutex_destroy(&rwlock->mutex);
    pthread_cond_destroy(&rwlock->readers_cond);
    pthread_cond_destroy(&rwlock->writers_cond);

    free(rwlock); // Free the memory used by the reader-writer lock
    *l = NULL; // Set the pointer to the reader-writer lock to NULL
}

// Acquire the reader lock
void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    // Wait while there are writers or the priority is set to WRITERS and there are readers
    while ((rw->writers > 0) || (rw->priority == WRITERS && rw->readers > 0)) {
        pthread_cond_wait(&rw->readers_cond, &rw->mutex);
    }

    rw->readers++; // Increment the number of readers
    pthread_mutex_unlock(&rw->mutex);
}

// Release the reader lock
void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);
    rw->readers--; // Decrement the number of readers

    if (rw->readers == 0) {
        pthread_cond_signal(
            &rw->writers_cond); // Signal a waiting writer if there are no more readers
    }

    pthread_mutex_unlock(&rw->mutex);
}

// Acquire the writer lock
void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    // Wait while there are readers or writers
    while ((rw->readers > 0) || (rw->writers > 0)) {
        pthread_cond_wait(&rw->writers_cond, &rw->mutex);
    }

    rw->writers++; // Increment the number of writers
    pthread_mutex_unlock(&rw->mutex);
}

// Release the writer lock
void writer_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);
    rw->writers--; // Decrement the number of writers

    if (rw->priority == N_WAY && rw->writers == 0) {
        pthread_cond_broadcast(
            &rw->readers_cond); // Broadcast to waiting readers for N-WAY priority
    } else {
        pthread_cond_signal(&rw->writers_cond); // Signal a waiting writer
    }

    pthread_mutex_unlock(&rw->mutex);
}
