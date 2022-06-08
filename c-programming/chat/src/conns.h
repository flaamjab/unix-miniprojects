#ifndef CONNS_H
#define CONNS_H

#define MAX_CONNECTIONS 4

typedef struct _Connection Connection;
typedef int Socket;
typedef void ActionF(
    Connection* original,
    Connection* receiver,
    void* data
);
typedef enum _ReadSource {
    READ_SOURCE_SERVER,
    READ_SOURCE_CLIENT
} ReadSource;

ReadSource await_read();
void set_server_sock(Socket socket);
int sock_is_readable(Socket sock);
const char* conn_name(const Connection* conn);
Connection* conn_by_name(const char* name);
int conn_socket(const Connection* conn);
int conn_new(const char* name, int socket);
void conn_drop_by_name(const char* name);
void conns_collapse();
void conns_for_all(Connection* originator, void* data, ActionF* action);
size_t num_active_conns();

#endif
