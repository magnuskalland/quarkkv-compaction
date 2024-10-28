#include "prepare.h"

#include <cstring>

int prepare_fs(Config* config)
{
    int ok;
    int len = 256;
    char cmd[len];

    memset(cmd, 0, len);
    std::snprintf(cmd, len, "mkdir -p %s %s\n", config->wdir.c_str(),
                  config->ddir.c_str());

    ok = system(cmd);
    if (ok != 0) {
        goto syserr;
    }

    memset(cmd, 0, len);
    std::snprintf(cmd, len, "rm -rf %s/*\n", config->wdir.c_str());

    ok = system(cmd);
    if (ok != 0) {
        goto syserr;
    }

    if (config->mode == LOAD) {
        memset(cmd, 0, len);
        std::snprintf(cmd, len, "rm -rf %s/*\n", config->ddir.c_str());

        ok = system(cmd);
        if (ok != 0) {
            goto syserr;
        }
        return 0;
    }

    memset(cmd, 0, len);
    printf("Copying all from %s to %s\n", config->ddir.c_str(), config->wdir.c_str());
    std::snprintf(cmd, len, "cp %s/* %s/\n", config->ddir.c_str(), config->wdir.c_str());

    ok = system(cmd);
    if (ok != 0) {
        goto syserr;
    }

    return 0;
syserr:
    perror("system");
    return -1;
}