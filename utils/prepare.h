#pragma once

#include <cstring>

#include "../include/config.h"

int prepare_fs(Config* config)
{
    int ok;
    int len = 256;
    char cmd[len];

    if (config->engine == FS) {
        memset(cmd, 0, len);
        std::snprintf(cmd, len, "rm -rf %s/*\n", config->fs_dbdir.c_str());

        ok = system(cmd);
        if (ok != 0) {
            goto syserr;
        }
        return 0;
    }

    return 0;
syserr:
    perror("system");
    return -1;
}