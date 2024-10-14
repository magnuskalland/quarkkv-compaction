#include "FSLoaderWorker.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "config.h"

FSLoaderWorker::FSLoaderWorker(Config *config, std::atomic<int> *counter) : Loader(config, counter) {}

void FSLoaderWorker::Work() {
    int seq, fd, ok;
    uint64_t offset = 0, smallest = ~0, biggest = 0, left;

    seq = counter_->fetch_add(1);
    while (seq < config_->nSstFiles) {
        std::string filename = getSstFilename(seq);

        fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0777);
        if (fd == -1) {
            perror("open");
            return;
        }

        ok = writeBuffer(fd, &smallest, &biggest);
        if (ok != 0) {
            return;
        }

        struct stat stat;
        ok = fstat(fd, &stat);
        assert(stat.st_size == SSTFILE_SIZE);

        ok = close(fd);
        if (ok != 0) {
            perror("close");
            return;
        }

        seq = counter_->fetch_add(1);
    }
}

int FSLoaderWorker::storageSystemWrite(int fd, const void *buf, size_t n) { return write(fd, buf, n); }
int FSLoaderWorker::storageSystemSync(int fd) { return fsync(fd); }

std::string FSLoaderWorker::getSstFilename(int sequence) {
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << sequence;
    return config_->directory + "/" + ss.str() + ".sst";
}