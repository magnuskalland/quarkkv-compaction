#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>

// Defaults
#define DEFAULT_PREPOPULATE_SIZE (16 * (DEFAULT_SST_FILE_SIZE / BLOCK_SIZE))  // ~16
#define DEFAULT_READ_SIZE 0
#define DEFAULT_WRITE_SIZE 0

#define DEFAULT_LEVELS 7
#define DEFAULT_FANOUT 10
#define DEFAULT_KEY_SIZE 56
#define DEFAULT_PRACTICAL_KEY_SIZE 56
#define DEFAULT_SST_FILE_SIZE ((long)1024 * 1024 * 64)  // 64 MiB
#define DEFAULT_LEVEL0_MAX_SIZE 1

#define BLOCK_SIZE 4096

enum mode { MANUAL = 0, YCSB };

enum storage_engine {
    FS = 0,
    QUARKSTORE,
};

enum compaction_picker {
    ALL = 0,
    ONE,
};

struct Config {
    /* mutable */
    enum storage_engine engine;
    enum mode mode = MANUAL;
    std::string ycsb_workload_path;
    std::string fs_dbdir;
    uint64_t prepopulate_size = DEFAULT_PREPOPULATE_SIZE;
    uint64_t read_size = DEFAULT_READ_SIZE;
    uint64_t write_size = DEFAULT_WRITE_SIZE;
    enum compaction_picker cp = ONE;
    uint32_t n_levels = DEFAULT_LEVELS;
    uint32_t fanout = DEFAULT_FANOUT;
    uint32_t key_size = DEFAULT_KEY_SIZE;
    uint32_t practical_key_size = DEFAULT_PRACTICAL_KEY_SIZE;
    uint32_t level0_max_size = DEFAULT_LEVEL0_MAX_SIZE;
    uint64_t sst_file_size = DEFAULT_SST_FILE_SIZE;

    /* immutable */
    uint32_t ts_size = 26;
    uint64_t quarkstore_current_aid = 1000;
    uint64_t quarkstore_manifest_aid_start = 1001;
    uint64_t quarkstore_sst_aid_start = 10000;

    uint32_t value_size()
    {
        assert(key_size < BLOCK_SIZE);
        return BLOCK_SIZE - key_size;
    }

    uint32_t kv_size()
    {
        return value_size() + key_size;
    }

    uint32_t index_block_entry_size()
    {
        return key_size + sizeof(size_t);
    }

    uint32_t maxSizeOfLevel(uint32_t level)
    {
        return pow(fanout, level) * level0_max_size;
    }

    std::string ToString()
    {
        char buffer[1024];
        std::sprintf(buffer,
                     R"(Config
    %-20s %s
    %-20s %s
    %-20s %s
    %-20s %s
    %-20s %ld (~%ld SSTs)
    %-20s %ld KV pairs
    %-20s %ld KV pairs
    %-20s %s
    %-20s %d
    %-20s %d
    %-20s %d bytes
    %-20s %d bytes
    %-20s %d SST file
    %-20s %lu MiB
    )",
                     "Engine:", engine == FS ? "FS" : "QuarkStore",
                     "Mode:", mode == MANUAL ? "Manual" : "YCSB",
                     "YCSB workload:", ycsb_workload_path.c_str(),
                     "File system DB directory:", fs_dbdir.c_str(),
                     "Prepopulate size:", prepopulate_size,
                     ((prepopulate_size * kv_size()) / sst_file_size),
                     "Read size:", read_size, "Write size:", write_size,
                     "Compaction picker:", cp == ALL ? "All" : "One", "Levels:", n_levels,
                     "Fanout:", fanout, "Key size:", key_size,
                     "Value size:", value_size(), "Level 0 max size:", level0_max_size,
                     "SST file size:", sst_file_size >> 20);
        return std::string(buffer);
    }
};