#pragma once

#include <atomic>

#include "KeyGenerator.h"
#include "config.h"

#define LOADER_THREADS 1

class Loader {
   public:
    Loader(Config *config, std::atomic<int> *counter);
    virtual void Work() = 0;

   protected:
    int writeBuffer(int handler, uint64_t *smallest, uint64_t *biggest);
    virtual int storageSystemWrite(int handler, const void *buf, size_t n) = 0;
    virtual int storageSystemSync(int handler) = 0;
    KeyGenerator keyGenerator_;
    Config *config_;
    std::atomic<int> *counter_;
};
