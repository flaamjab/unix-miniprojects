#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "conns.h"

typedef struct _Connection {
    struct _Connection* next;
    char* name;
    Socket socket;
} Connection;

Socket server_socket = 0;
fd_set read_set;

#define HASHSIZE 101

static Connection* hashtab[HASHSIZE];
static size_t n_conns = 0;

unsigned hash(const char* s) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

void set_server_sock(Socket socket) {
    server_socket = socket;

    FD_ZERO(&read_set);
    FD_SET(server_socket, &read_set);
}

void conns_update(int* nfds) {
    *nfds = server_socket;

    fd_set readable_sockets;
    memcpy(&readable_sockets, &read_set, sizeof(fd_set));

    FD_ZERO(&read_set);
    FD_SET(server_socket, &read_set);

    for (int h = 0; h < HASHSIZE; h++) {
        Connection* conn = hashtab[h];
        while (conn != NULL) {
            if (conn->socket > *nfds) {
                *nfds = conn->socket;
            }

            FD_SET(conn->socket, &read_set);

            conn = conn->next;
        }
    }
}

ReadSource await_read() {
    int nfds = 0;
    conns_update(&nfds);
    select(nfds+1, &read_set, NULL, NULL, NULL);

    if (FD_ISSET(server_socket, &read_set)) {
        return READ_SOURCE_SERVER;
    } else {
        return READ_SOURCE_CLIENT;
    }
}

const char* conn_name(const Connection* conn) {
    return conn->name;
}

int conn_socket(const Connection* conn) {
    return conn->socket;
}

Connection* conn_by_name(const char* name) {
    Connection* np = hashtab[hash(name)];
    while (np != NULL) {
        if (strcmp(name, np->name) == 0) {
            return np;
        }
        np = np->next;
    }
        
    return NULL;
}

int conn_new(const char* name, int socket) {
    if (n_conns < MAX_CONNECTIONS) {
        unsigned hashval;
        Connection* np = conn_by_name(name);
        if (np == NULL) {
            np = (Connection*)malloc(sizeof(*np));
            if (np == NULL) {
                return -1;
            }
            np->name = strdup(name);
            if (np->name == NULL) {
                return -1;
            }

            hashval = hash(name);
            np->next = hashtab[hashval];
            hashtab[hashval] = np;

            np->socket = socket;
            n_conns++;

            return 0;
        } else {
            return -2;
        }
    } else {
        return -3;
    }
}

void conn_drop(Connection* conn) {
    close(conn->socket);
    free(conn->name);
    free(conn);
}

void conn_drop_by_name(const char* name) {
    unsigned hashval = hash(name);
    Connection* cur = hashtab[hashval];
    Connection* prev = NULL;

    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) {
            
            if (prev != NULL) {
                prev->next = cur->next;
            } else {
                hashtab[hashval] = NULL;
            }

            conn_drop(cur);

            n_conns--;
            return;
        }

        prev = cur;
        cur = cur->next;
    }
}

void conns_collapse() {
    for (int h = 0; h < HASHSIZE; h++) {
        Connection* conn = hashtab[h];
        while (conn != NULL) {
            close(conn->socket);
            Connection* next = conn->next;
            free((void*)conn);
            conn = next;
        }
        hashtab[h] = NULL;
    }
}

int sock_is_readable(Socket sock) {
    return FD_ISSET(sock, &read_set);
}

void conns_for_all(Connection* originator, void* data, ActionF* action) {
    for (int h = 0; h < HASHSIZE; h++) {
        Connection* conn = hashtab[h];
        while (conn != NULL) {
            Connection* next = conn->next;
            action(originator, conn, data);
            conn = next;
        }
    }
}

size_t num_active_conns() {
    return n_conns;
}
