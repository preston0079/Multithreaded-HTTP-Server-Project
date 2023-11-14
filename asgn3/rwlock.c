#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#include "rwlock.h"

#include <assert.h>

// Referenced textbook: Tom Anderson and Mike Dahlin’s Operating Systems: Principle and Practice Volume II: Concurency

typedef struct rwlock {
    int reader; // Number of current readers
    int writer; // Number of current writers
    int waiting_readers;
    int waiting_writers;

    int n; // N-WAY parameter
    PRIORITY priority; // Reader-writer lock priority (READERS, WRITERS, N_WAY)

    pthread_mutex_t mutex; // Mutex
    pthread_cond_t reader_cond_var; // Condition variable for readers
    pthread_cond_t writer_cond_var; // Condition variable for writers
} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    // Allocate memory for rwlock
    rwlock_t *rwlock = (rwlock_t *) malloc(sizeof(rwlock_t));
    // Assert
    if (rwlock == NULL) {
        return NULL;
    }

    rwlock->reader = 0; // Set current/active readers to 0
    rwlock->writer = 0; // Set current/active writers to 0

    rwlock->waiting_readers = 0; // Set waiting readers to 0
    rwlock->waiting_writers = 0; // set waiting writers to 0

    rwlock->n = n; // Set N-WAY parameter
    rwlock->priority = p; // Set priority

    // Initialize the mutex and condition variables
    pthread_mutex_init(&rwlock->mutex, NULL);
    pthread_cond_init(&rwlock->reader_cond_var, NULL);
    pthread_cond_init(&rwlock->writer_cond_var, NULL);

    return rwlock;
}

void rwlock_delete(rwlock_t **rw) {

    if (rw != NULL || *rw != NULL) {
        rwlock_t *rwlock = *rw;

        // Destroy the mutex and condition variables
        pthread_mutex_destroy(&rwlock->mutex);
        pthread_cond_destroy(&rwlock->reader_cond_var);
        pthread_cond_destroy(&rwlock->writer_cond_var);

        free(rwlock); // Free the memory for rwlock
        *rw = NULL; // Set the pointer to NULL
    }
}

void reader_lock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    rw->waiting_readers++;
    while (rw->writer > 0 || rw->waiting_writers > 0) {
        pthread_cond_wait(&rw->reader_cond_var, &rw->mutex);
    }
    rw->waiting_readers--;
    rw->reader++;

    pthread_mutex_unlock(&rw->mutex); // Unlock mutex
}

void reader_unlock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    rw->reader--;
    if (rw->reader == 0 && rw->waiting_writers > 0) {
        pthread_cond_signal(&rw->writer_cond_var);
    }

    pthread_mutex_unlock(&rw->mutex); // Unlock mutex
}

void writer_lock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    rw->waiting_writers++;
    while (rw->writer > 0 || rw->reader > 0) {
        pthread_cond_wait(&rw->writer_cond_var, &rw->mutex);
    }
    rw->waiting_writers--;
    rw->writer++;

    pthread_mutex_unlock(&rw->mutex); // Unlock mutex
}

void writer_unlock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex); // Lock the mutex

    rw->writer--;
    assert(rw->writer == 0);
    if (rw->waiting_writers > 0) {
        pthread_cond_signal(&rw->writer_cond_var);
    } else {
        pthread_cond_broadcast(&rw->reader_cond_var);
    }

    pthread_mutex_unlock(&rw->mutex); // Unlock the mutex
}
