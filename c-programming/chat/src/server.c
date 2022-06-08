#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <signal.h>
#include "cfg.h"
#include "conns.h"

typedef struct _SocketData {
    int sock;
    char* path;
} SocketData;
SocketData socket_data;

int socket_drop(const SocketData* socket_data) {
    close(socket_data->sock);
    unlink(socket_data->path);
}

void handle_interrupt(int dummy) {
    socket_drop(&socket_data);
    exit(EXIT_SUCCESS);
}

void send_message(
    Connection* origin,
    Connection* receiver,
    void* msg
) {
    if (origin != receiver) {
        const char* name = conn_name(origin);
        Socket sock = conn_socket(receiver);
        write(sock, "MESG", CMD_LENGTH);
        write(sock, name, MAX_NAME_LENGTH);
        write(sock, (char*)msg, MESSAGE_LENGTH);
    }
}

int receive_message(Socket sock, char* name, char* msg) {
    int n = recv(sock, name, MAX_NAME_LENGTH, MSG_DONTWAIT);
    if (n > 0) {
        name[n-1] = '\0';
        if (conn_by_name(name) == NULL) {
            return -1;
        }
    } else {
        return -2;
    }

    n = recv(sock, msg, MESSAGE_LENGTH, MSG_DONTWAIT);
    if (n > 0) {
        msg[n-1] = '\0';
    } else {
        return -2;
    }

    return 0;
}

void command(
    Connection* original,
    Connection* current,
    void* data
) {
    int sock = conn_socket(current);
    if (sock_is_readable(sock)) {
        char cmd[CMD_LENGTH];
        int n = read(sock, cmd, CMD_LENGTH);
        if (n == 0) {
            printf("#%.3d has disconnected\n", sock);
            const char* name = conn_name(current);
            conn_drop_by_name(name);
        } else {
            cmd[CMD_LENGTH-1] = 0;
            printf("#%.3d issued %s\n", sock, cmd);
            if (strcmp(cmd, "MESG") == 0) {
                char msg[MESSAGE_LENGTH];
                char name[MAX_NAME_LENGTH];
                int err = receive_message(sock, name, msg);
                if (err == 0) {
                    conns_for_all(current, (void*)msg, send_message);
                } else if (err == -1) {
                    printf("Client has sent a broken message\n");
                } else if (err == -2) {
                    printf("Client failed to authenticate\n");
                }
            } else if (strcmp(cmd, "EXIT") == 0) {
                char name[MAX_NAME_LENGTH];
                read(sock, name, MAX_NAME_LENGTH);
                conn_drop_by_name(name);
            }
            else {
                printf("Client issued an invalid command (%s)\n", cmd);
            }
        }
    }
}

int handshake(int sock) {
    char buf[5];
    memset(buf, 5, sizeof(char));
    read(sock, buf, 4);
    if (strcmp("CONN", buf) == 0) {
        char name[MAX_NAME_LENGTH];
        read(sock, name, MAX_NAME_LENGTH);
        int err = conn_new(name, sock);
        if (err == 0) {
            write(sock, "SUCC", 4);
            return 0;
        } else {
            printf("Rejected connection due to the following error (%d):\n", err);
            if (err == -2) {
                printf(
                    "A user with the name \"%s\" is already connected\n",
                    name
                );
            } else if (err == -3) {
                printf("The server is full\n");
            } else {
                perror("system");
            }

            write(sock, "FAIL", 4);

            return -1;
        }
    } else {
        printf("#%.3d does not handshake (sent %s).\n", sock, buf);
        close(sock);
        return -1;
    }
}

void await_messages(int sock) {
    int err = listen(sock, 3);
    if (err == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Ready.\n");
    for (;;) {
        ReadSource src = await_read();

        if (src == READ_SOURCE_SERVER) {
            printf("Incoming connection...\n");
            int in_sock = accept(sock, NULL, NULL);
            int err = handshake(in_sock);
            if (err == 0) {
                printf("#%.3d acknowledged", in_sock);
            } else {
                printf("#%.3d declined", in_sock);
            }
            int n_conns = num_active_conns();
            printf(" (%d/%d)\n", n_conns, MAX_CONNECTIONS);
        } else if (src == READ_SOURCE_CLIENT) {
            conns_for_all(0, NULL, command);
        }
    }
}

int socket_new(const char* path) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, PATH_LENGTH-1);

    int err = bind(
        sock,
        (const struct sockaddr*)&addr,
        sizeof(addr)-1
    );

    if (err == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int main(int argc, const char** argv) {
    char path[PATH_LENGTH];
    int err = socket_name("socket.cfg", path);
    if (err) {
        fprintf(stderr, "Error occurred when reading the file.\n");
    } else {
        printf("Socket path is %s\n", path);
    }

    int sock = socket_new(path);
    socket_data.sock = sock;
    socket_data.path = path;

    set_server_sock(sock);

    signal(SIGINT, handle_interrupt);

    await_messages(sock);

    printf("Shutting down...\n");
    socket_drop(&socket_data);
}
