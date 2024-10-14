#pragma once

#include <iomanip>
#include <sstream>

#include "Loader.h"

class FSLoaderWorker : public Loader {
   public:
    FSLoaderWorker(Config *config, std::atomic<int> *counter);
    void Work() override;

   protected:
    int storageSystemWrite(int fd, const void *buf, size_t n) override;
    int storageSystemSync(int fd) override;

   private:
    std::string getSstFilename(int sequence);
};