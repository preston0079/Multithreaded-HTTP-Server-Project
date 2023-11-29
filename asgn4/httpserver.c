#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "asgn2_helper_funcs.h"
#include "connection.h"
//#include "debug.h"
#include "queue.h"
#include "response.h"
#include "request.h"
#include "rwlock.h"

// Work used from previous class
// Help recieved from classmate, all help shown on whiteboard no code was shared

typedef struct rwlockHTNodeObj {
    char *uri;
    rwlock_t *rwlock;
    struct rwlockHTNodeObj *next;
} rwlockHTNodeObj;

typedef rwlockHTNodeObj *rwlockHTNode;

typedef struct rwlockHTObj {
    rwlock_t **rwlocks;
} rwlockHTObj;

typedef rwlockHTObj *rwlockHT;

typedef struct Table {
    struct rwlockHTNodeObj **buckets;
    size_t num_buckets;
} Table;

typedef struct ThreadObj {
    pthread_t thread;
    int id;
    // rwlockHT *rwlockHT;
    Table *rwlockHT;
    queue_t *queue;
} ThreadObj;

typedef ThreadObj *Thread;

queue_t *queue;
// rwlockHT *rwlock_table;

pthread_mutex_t mutex;
pthread_mutex_t table_mutex;
pthread_mutex_t audit_lock;

Table *create_table(size_t num_buckets) {
    Table *t = calloc(1, sizeof(Table)); //allocate space for the table
    t->buckets = calloc(num_buckets, sizeof(rwlockHTNodeObj *)); //allocate space for the array
    t->num_buckets = num_buckets; //set size of the hashtable
    return t;
}

Table *hashtable;

unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while (*str != '\0') {
        c = *str;
        hash = ((hash << 5) + hash) + (unsigned char) c; // hash * 33 + c
        str++;
    }
    return hash;
}

rwlockHTNodeObj *add_node_to_list(char *uri, rwlock_t *rwlock, rwlockHTNodeObj *bucket) {
    rwlockHTNodeObj *new_node;
    new_node = calloc(1, sizeof(rwlockHTNodeObj));
    new_node->uri = strdup(uri);
    new_node->rwlock = rwlock;
    new_node->next = bucket;
    return new_node;
}

void add_to_table(char *uri, rwlock_t *rwlock, Table *table) {
    unsigned long hashvalue = hash(uri);
    size_t index = hashvalue % table->num_buckets;
    rwlockHTNodeObj *linkedlist = (table->buckets)[index];
    while (linkedlist && strcmp((linkedlist->uri), uri) != 0) {
        linkedlist = linkedlist->next;
    }
    if (linkedlist != NULL) {
        free(linkedlist->rwlock);
        linkedlist->rwlock = rwlock;
    } else {
        (table->buckets)[index] = add_node_to_list(uri, rwlock, (table->buckets)[index]);
    }
    return;
}

rwlockHTNodeObj *find_URI(char *uri, Table *table) {
    size_t index = hash(uri) % (table->num_buckets);
    rwlockHTNodeObj *linkedlist = (table->buckets)[index];
    while (linkedlist != NULL) {
        if (strcmp(linkedlist->uri, uri) == 0) {
            return linkedlist;
        }
        linkedlist = linkedlist->next;
    }
    return NULL;
}

void handle_connection(int);

void worker_thread(void *args);

void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *);

int main(int argc, char **argv) {

    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Use getopt to get the number of threads
    size_t t = 4; //thread count (default 4)
    int opt;

    // Parse command-line options using getopt (used previous class getopt format)
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't': t = atoi(optarg); break;
        default:
            fprintf(stderr, "Usage: %s -t <number_of_threads> <port>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[optind], &endptr, 10);
    if (endptr && *endptr != '\0') {
        warnx("invalid port number: %s", argv[1]);
        return EXIT_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    hashtable = create_table(100);

    // Thread threads[t];
    ThreadObj *threads[t];
    Table *rwlock_ht = create_table(100);
    queue_t *queue = queue_new(t);
    pthread_t *thread_ids = malloc(sizeof(pthread_t) * t);

    // Create worker threads
    for (size_t i = 0; i < t; ++i) {
        threads[i] = malloc(sizeof(ThreadObj));
        threads[i]->id = i;
        threads[i]->rwlockHT = rwlock_ht;
        threads[i]->queue = queue;

        pthread_create(
            &thread_ids[i], NULL, (void *(*) (void *) ) worker_thread, (void *) threads[i]);
    }

    while (1) {
        uintptr_t connfd = listener_accept(&sock);
        queue_push(queue, (void *) connfd);
    }

    return EXIT_SUCCESS;
}

void worker_thread(void *args) {

    Thread thread = (Thread) args;
    //int id = thread->id;
    //rwlockHT *rwlock_ht = thread->rwlockHT;
    queue_t *queue = thread->queue;

    while (1) {
        uintptr_t connfd = 0;
        queue_pop(queue, (void **) &connfd);
        handle_connection(connfd);
        close(connfd);
    }
}

void handle_connection(int connfd) {

    conn_t *conn = conn_new(connfd);

    const Response_t *res = conn_parse(conn);

    if (res != NULL) {
        conn_send_response(conn, res);
    } else {
        // debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);
        if (req == &REQUEST_GET) {
            handle_get(conn);
        } else if (req == &REQUEST_PUT) {
            handle_put(conn);
        } else {
            handle_unsupported(conn);
        }
    }
}

void handle_get(conn_t *conn) {

    pthread_mutex_lock(&mutex);

    const Response_t *res = NULL;

    char *uri = conn_get_uri(conn);
    // debug("GET request no implemented. But we want to get %s", uri);
    bool existed = access(uri, F_OK) == 0;

    pthread_mutex_lock(&table_mutex);
    rwlock_t *rwlock;
    rwlockHTNodeObj *node;
    node = find_URI(uri, hashtable);
    if (node != NULL) {
        rwlock = node->rwlock;
    } else {
        rwlock = rwlock_new(N_WAY, 1);
        add_to_table(uri, rwlock, hashtable);
    }
    pthread_mutex_unlock(&table_mutex);

    reader_lock(rwlock);

    // 1. Open the file.
    int fd = open(uri, O_RDONLY);
    pthread_mutex_unlock(&mutex);

    if (fd < 0) {
        // debug("%s: %d", uri, errno);

        if (errno == EACCES) {
            res = &RESPONSE_FORBIDDEN;
            conn_send_response(conn, res);
        } else if (errno == ENOENT) {
            res = &RESPONSE_NOT_FOUND;
            conn_send_response(conn, res);
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            conn_send_response(conn, res);
        }
    }

    // 2. Get the size of the file.
    struct stat st;
    if (fstat(fd, &st) < 0) {
        res = &RESPONSE_INTERNAL_SERVER_ERROR;
        conn_send_response(conn, res);
    }

    // 3. Check if the file is a directory.
    if (S_ISDIR(st.st_mode)) {
        res = &RESPONSE_FORBIDDEN; // Assuming directories are forbidden
        conn_send_response(conn, res);
    }

    // 4. Send the file
    if (res == NULL && existed) {
        res = conn_send_file(conn, fd, st.st_size);
        res = &RESPONSE_OK;
    }

    fprintf(stderr, "GET,%s,%hu,%s\n", uri, response_get_code(res),
        conn_get_header(conn, "Request-Id"));
    reader_unlock(rwlock);
    close(fd);
}

void handle_put(conn_t *conn) {

    pthread_mutex_lock(&mutex);

    const Response_t *res = NULL;

    char *uri = conn_get_uri(conn);
    // debug("PUT request no implemented. But we want to get %s", uri);

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;

    pthread_mutex_lock(&table_mutex);
    rwlock_t *rwlock;
    rwlockHTNodeObj *node;
    node = find_URI(uri, hashtable);
    if (node != NULL) {
        rwlock = node->rwlock;
    } else {
        rwlock = rwlock_new(N_WAY, 1);
        add_to_table(uri, rwlock, hashtable);
    }
    pthread_mutex_unlock(&table_mutex);

    writer_lock(rwlock);

    // Open the file.
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0600);

    pthread_mutex_unlock(&mutex);

    if (fd < 0) {
        // debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            conn_send_response(conn, res);
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            conn_send_response(conn, res);
        }
    }

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

    fprintf(stderr, "PUT,%s,%hu,%s\n", uri, response_get_code(res),
        conn_get_header(conn, "Request-Id"));
    conn_send_response(conn, res);
    writer_unlock(rwlock);
    close(fd);
}

void handle_unsupported(conn_t *conn) {
    // debug("handling unsupported request");

    // send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
}
