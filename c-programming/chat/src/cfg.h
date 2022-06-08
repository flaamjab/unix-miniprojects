#ifndef CFG_H
#define CFG_H

#include <stdlib.h>

#define CMD_LENGTH (4+1)
#define MAX_NAME_LENGTH 32
#define MESSAGE_LENGTH 32
#define PATH_LENGTH 108

int socket_name(const char* cfg_file, char* name);

#endif
