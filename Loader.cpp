#include "Loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <cstring>

#include "config.h"

Loader::Loader(Config *config, std::atomic<int> *counter)
    : config_(config), counter_(counter), keyGenerator_(KEY_SIZE) {}

int Loader::writeBuffer(int fd, uint64_t *smallest, uint64_t *biggest) {
    uint64_t bufsize = KVPAIR_SIZE * 64;
    uint64_t left;
    uint64_t offset;
    char buf[bufsize];
    int keyValue, ok;

    left = SSTFILE_SIZE;
    while (left > 0) {
        memset(buf, '?', bufsize);
        offset = 0;
        while (offset < bufsize) {
            std::string key = keyGenerator_.GenerateRand(&keyValue);
            *smallest = keyValue < *smallest ? keyValue : *smallest;
            *biggest = keyValue > *biggest ? keyValue : *biggest;
            memcpy(&buf[offset], key.c_str(), KEY_SIZE);
            offset = offset + KVPAIR_SIZE;
        }
        assert(offset == bufsize);

        ok = storageSystemWrite(fd, buf, bufsize);
        if (ok != bufsize) {
            perror("write");
            return -1;
        }

        left = left - bufsize;
    }

    return 0;
}
