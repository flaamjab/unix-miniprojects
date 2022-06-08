#include <stdio.h>
#include "cfg.h"

int socket_name(const char* cfg_file, char* name) {
    FILE* cfg_fid = fopen(cfg_file, "r");

    if (!cfg_file) {
        return 1;
    }

    size_t n = fread(name, sizeof(char), PATH_LENGTH-1, cfg_fid);
    name[n] = 0;

    return 0;
}

