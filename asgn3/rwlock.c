#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#include "rwlock.h"

typedef struct rwlock {
    int reader; // Number of current readers
    int writer; // Number of current writers
    int n; // N-WAY parameter
    PRIORITY priority; // Reader-writer lock priority (READERS, WRITERS, N_WAY)
    pthread_mutex_t mutex; // Mutex
    pthread_cond_t reader_cond_var; // Condition variable for readers
    pthread_cond_t writer_cond_var; // Condition variable for writers
} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    // Allocate memory for rwlock
    rwlock_t *rwlock = (rwlock_t *) malloc(sizeof(rwlock_t));
    if (rwlock == NULL) {
        return NULL;
    }

    rwlock->reader = 0; // Set current readers to 0
    rwlock->writer = 0; // Set current writers to 0
    rwlock->n = n; // Set N-WAY parameter
    rwlock->priority = p; // Set priority

    // Initialize the mutex and condition variables
    pthread_mutex_init(&rwlock->mutex, NULL);
    pthread_cond_init(&rwlock->reader_cond_var, NULL);
    pthread_cond_init(&rwlock->writer_cond_var, NULL);

    return rwlock;
}

void rwlock_delete(rwlock_t **l) {
    if (l == NULL || *l == NULL) {
        return;
    }

    rwlock_t *rwlock = *l;

    // Destroy the mutex and condition variables
    pthread_mutex_destroy(&rwlock->mutex);
    pthread_cond_destroy(&rwlock->reader_cond_var);
    pthread_cond_destroy(&rwlock->writer_cond_var);

    free(rwlock); // Free the memory for rwlock
    *l = NULL; // Set the pointer to NULL
}

void reader_lock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    // Wait while there are writers OR (the priority is set to WRITERS and there are readers)
    while ((rw->writer > 0) || (rw->priority == WRITERS && rw->reader > 0)) {
        pthread_cond_wait(&rw->reader_cond_var, &rw->mutex);
    }

    rw->reader++; // Increment the number of readers
    pthread_mutex_unlock(&rw->mutex); // Unlock mutex
}

void reader_unlock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    rw->reader--; // Decrement the number of readers

    // Signal a waiting writer if there are no more readers
    if (rw->reader == 0) {
        pthread_cond_signal(&rw->writer_cond_var);
    }

    pthread_mutex_unlock(&rw->mutex); // Unlock mutex
}

void writer_lock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    // Wait while there are readers OR writers
    while ((rw->reader > 0) || (rw->writer > 0)) {
        pthread_cond_wait(&rw->writer_cond_var, &rw->mutex);
    }

    rw->writer++; // Increment the number of writers

    pthread_mutex_unlock(&rw->mutex); // Unlock mutex
}

void writer_unlock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    rw->writer--; // Decrement the number of writers

    // Broadcast to waiting readers for N-WAY priority
    if (rw->priority == N_WAY && rw->writer == 0) {
        // Professor Quinn mentioned after lecture that broadcast can be used
        pthread_cond_broadcast(&rw->reader_cond_var);
    } else {
        pthread_cond_signal(&rw->writer_cond_var); // Signal a waiting writer
    }

    pthread_mutex_unlock(&rw->mutex); // Unlock the mutex
}
