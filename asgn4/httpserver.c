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

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "response.h"
#include "request.h"
#include "rwlock.h"

typedef struct rwlockHTNodeObj {
    rwlock_t *rwlock;
    rwlockHTNode *next;
} rwlockHTNodeObj;

typedef struct rwlockHTObj {

} rwlockHTObj;

typedef rwlockHTObj *rwlockHT;

typedef struct ThreadObj {
    pthread_t thread;
    int id;
    rwlockHT_t *rwlockHT_t;
} ThreadObj;

typedef ThreadObj *Thread;

void handle_connection(int);

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
    int t; //thread count (default 4)

    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[1], &endptr, 10);
    if (endptr && *endptr != '\0') {
        warnx("invalid port number: %s", argv[1]);
        return EXIT_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    Thread threads[t];
    rwlockHT *rwlock_ht = new_rwlockHT(t);

    // pthread_t *tids = malloc(sizeof(pthread_t * t));

    // Create worker threads
    for (int i = 0; i <= t; ++i) {
        threads[i] = malloc(sizeof(ThreadObj));
        threads[i]->if = i;
        threads[i]->rwlockHT = rwlock_ht;

        pthread_create(tids + 1, NULL, (void *(*) (void *) ) handle_connection, NULL);
    }

    while (1) {
        uintptr_t connfd = listener_accept(&sock);
        queue_push(queue, (void *) connfd);
    }

    return EXIT_SUCCESS;
}

void worker_thread(void) {

    Thread thread = (Thread) args;
    int id = thread->id;
    rwlockHT *rwlock_ht = thread->rwlockHT;

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
        debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);
        if (req == &REQUEST_GET) {
            handle_get(conn);
        } else if (req == &REQUEST_PUT) {
            handle_put(conn);
        } else {
            handle_unsupported(conn);
        }
    }

    // while(1){

    //     uintptr_t connfd = 0;
    //     queue_pop(queue, (void **) &connfd);

    //     conn_t * conn = conn_new(connfd);

    //     const Response_t *res = conn_parse(conn);

    //     if (res != NULL) {
    //         conn_send_resposne(conn, res);
    //     } else {
    //         debug("%s", conn_str(conn));
    //         const Request_t *req = conn_get_request(conn);
    //         if (req == &REQUEST_GET) {
    //             handle_get(conn);
    //         } else if (req == &REQUEST_PUT) {
    //             handle_put(conn);
    //         } else {
    //             handle_unsupported(conn);
    //         }
    //     }

    //     conn_delete(&conn);
    // }
}

void handle_get(conn_t *conn) {
}

void handle_push(conn_t *conn) {
}

void handle_unsupported(conn_t *conn) {
    debug("handling unsupported request");

    // send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
}
