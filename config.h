#pragma once

#include <string>

#define BLOCK_SIZE 4096
#define KEY_SIZE 1024
#define VALUE_SIZE (BLOCK_SIZE - KEY_SIZE)
#define KVPAIR_SIZE (KEY_SIZE + VALUE_SIZE)

#define DATASET_SIZE ((long)1024 * 1024 * 1024)  // 1 GiB
#define SSTFILE_SIZE ((long)1024 * 1024 * 64)    // 64 MiB
#define N_SSTFILES ((DATASET_SIZE) / SSTFILE_SIZE)

#define LEVELS 7

#define DIR_PATH_LEN 128

enum Mode {
    LOAD = 0,
    COMPACT,
};

enum StorageEngine {
    FS = 0,
    QUARKSTORE,
};

struct Config {
    enum Mode mode;
    enum StorageEngine engine;
    std::string directory;
    int nSstFiles = N_SSTFILES;
};
