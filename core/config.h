#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>

// Defaults
#define DEFAULT_LEVELS 7
#define DEFAULT_FANOUT 10
#define DEFAULT_KEY_SIZE 56
#define DEFAULT_PRACTICAL_KEY_SIZE 12
#define DEFAULT_SST_FILE_SIZE ((long)1024 * 1024 * 64)  // 64 MiB
#define DEFAULT_LEVEL0_MAX_SIZE 1

#define BLOCK_SIZE 4096

enum mode { COMPACT = 0, LOAD };

enum storage_engine {
    FS = 0,
    QUARKSTORE,
};

enum compaction_picker {
    ALL = 0,
    ONE,
};

struct Config {
    enum storage_engine engine;
    std::string ddir;
    std::string wdir;
    enum mode mode = COMPACT;
    uint32_t n_levels = DEFAULT_LEVELS;
    uint32_t fanout = DEFAULT_FANOUT;
    uint32_t key_size = DEFAULT_KEY_SIZE;
    uint32_t practical_key_size = DEFAULT_PRACTICAL_KEY_SIZE;
    uint32_t level0_max_size = DEFAULT_LEVEL0_MAX_SIZE;
    uint64_t sst_file_size = DEFAULT_SST_FILE_SIZE;
    enum compaction_picker cp = ONE;
    uint32_t ts_size = 26;
    uint32_t populateSize = 16 * (sst_file_size / kv_size());  // 16 full SSTs

    int quarkstore_current_aid = 0;
    int quarkstore_manifest_aid_start = 1;
    int quarkstore_sst_aid_start = 1000;

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

    std::string cwd()
    {
        return mode == LOAD ? ddir : wdir;
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
    %-20s %d
    %-20s %d
    %-20s %d bytes
    %-20s %d bytes
    %-20s %d SST file
    %-20s %lu MiB
    %-20s %s
    )",
                     "Engine:", engine == FS ? "FS" : "QuarkStore",
                     "Data directory:", ddir.c_str(), "Working directory:", wdir.c_str(),
                     "Mode:", mode == COMPACT ? "Compact" : "Load", "Levels:", n_levels,
                     "Fanout:", fanout, "Key size:", key_size,
                     "Value size:", value_size(), "Level 0 max size:", level0_max_size,
                     "SST file size:", sst_file_size >> 20,
                     "Compaction picker:", cp == ALL ? "All" : "One");
        return std::string(buffer);
    }
};