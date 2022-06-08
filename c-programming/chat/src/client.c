#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "cfg.h"

int socket_new(const char* path) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un name;
    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, path, PATH_LENGTH-1);

    int err = connect(
        sock,
        (const struct sockaddr*)&name,
        sizeof(name.sun_path)-1
    );

    if (err == -1) {
        fprintf(stderr, "The server is unreachable.\n");
        exit(EXIT_FAILURE);
    }

    return sock;
}

void strnreplace(char* str, size_t n, const char* old, char new) {
    for (int pos = 0; pos < n; pos++) {
        if (strchr(old, str[pos]) != NULL) {
            str[pos] = new;
        }
    }
}

char* username() {
    char* name = NULL;
    size_t size = 0;
    printf("Nickname?\n");
    for (;;) {
        printf("> ");
        int n = getline(&name, &size, stdin);
        if (n > MAX_NAME_LENGTH) {
            fprintf(stderr, "Name cannot exceed %d bytes\n", MAX_NAME_LENGTH);
        } else if (n <= 2) {
            fprintf(stderr, "Name must be longer than 1 character.\n");
        } else if (n <= 0) {
            printf("\nExiting...\n");
            return EXIT_SUCCESS;
        } else {
            name[n-1] = '\0';
            break;
        }
    }

    return name;
}

int handshake(int socket, const char* name) {
    write(socket, "CONN", 4);
    write(socket, name, MAX_NAME_LENGTH);

    char buf[5];
    memset(buf, 5, sizeof(char));
    read(socket, buf, 4);
    if (strcmp(buf, "SUCC") == 0) {
        return 0;
    } else if (strcmp(buf, "FAIL") == 0) {
        return -1;
    } else {
        return -2;
    }
}

int send_message(int sock, const char* name, const char* msg) {
    int n = write(sock, "MESG", CMD_LENGTH);
    n = write(sock, name, MAX_NAME_LENGTH);
    n = write(sock, msg, MESSAGE_LENGTH);

    if (n == -1) {
        return -1;
    } else {
        return 0;
    }
}

int receive_message(int sock, char* source_name, char* msg) {
    int n = recv(sock, source_name, MAX_NAME_LENGTH, MSG_DONTWAIT);
    if (n > 0) {
        strnreplace(source_name, n, "\n", '\0');
    } else {
        return -1;
    }

    n = recv(sock, msg, MESSAGE_LENGTH, MSG_DONTWAIT);
    if (n > 0) {
        strnreplace(msg, n, "\n", '\0');
    } else {
        return -1;
    }

    return 0;
}

void await_input(int sock, const char* name) {
    for (;;) {
        printf("%s> ", name);
        size_t size = 0;
        char* line = NULL;
        int n = getline(&line, &size, stdin);
        if (n > 1) {
            int err = send_message(sock, name, line);
            if (err != 0) {
                fprintf(stderr, "Error sending message.\n");
            }
        } else if (n <= 0) {
            printf("Exiting...\n");
            return;
        }
        free(line);
    }
}

void await_update(int sock, const char* name) {
    for (;;) {
        char cmd[CMD_LENGTH];
        int n = read(sock, cmd, CMD_LENGTH);
        if (n > 0) {
            cmd[n-1] = 0;
            if (strcmp("MESG", cmd) == 0) {
                char sender_name[MAX_NAME_LENGTH];
                char msg[MESSAGE_LENGTH];
                int err = receive_message(sock, sender_name, msg);
                if (err == 0) {
                    printf("\n%s: %s\n%s> ", sender_name, msg, name);
                    fflush(stdout);
                } else {
                    printf("A broken message passed by\n");
                }
            } else {
                printf("Server issued an unsupported command.\n");
            }
        } else {
            printf("Server has disconnected.\n");
            exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, const char** argv) {
    signal(SIGPIPE, SIG_IGN);

    char* name = username();

    char path[PATH_LENGTH];
    int err = socket_name("socket.cfg", path);
    if (err) {
        fprintf(stderr, "Error occurred when reading the file.\n");
    } else {
        printf("Socket path is %s\n", path);
    }

    int sock = socket_new(path);

    err = handshake(sock, name);
    if (err == 0) {
        printf("You are now connected.\n");
        if (fork() == 0) {
            await_update(sock, name);
        } else {
            await_input(sock, name);
        }
    } else if (err == -1) {
        printf("The server refused to maintain connection.\n");
    } else {
        printf("The server response was unexpected.\n");
    }

    write(sock, "EXIT", 4);
    write(sock, name, MAX_NAME_LENGTH);
    close(sock);
}
