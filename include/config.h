#pragma once

#include <cassert>
#include <format>
#include <sstream>
#include <string>

// Defaults
#define DEFAULT_LEVELS 7
#define DEFAULT_LEVEL_SIZE_MULTIPLIER 10
#define DEFAULT_KEY_SIZE 56
#define DEFAULT_SST_FILE_SIZE ((long)1024 * 1024 * 64)  // 64 MiB
#define DEFAULT_LEVEL0_MAX_SIZE 1

#define BLOCK_SIZE 4096

enum mode { COMPACT = 0, LOAD };

enum storage_engine {
    FS = 0,
    QUARKSTORE,
};

struct Config {
    enum storage_engine engine;
    std::string ddir;
    std::string wdir;
    enum mode mode = COMPACT;
    uint32_t n_levels = DEFAULT_LEVELS;
    uint32_t level_size_multiplier = DEFAULT_LEVEL_SIZE_MULTIPLIER;
    uint32_t key_size = DEFAULT_KEY_SIZE;
    uint32_t level0_max_size = DEFAULT_LEVEL0_MAX_SIZE;
    uint64_t sst_file_size = DEFAULT_SST_FILE_SIZE;

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
        return key_size + sizeof(uint64_t);
    }

    std::string cwd()
    {
        return mode == LOAD ? ddir : wdir;
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
    )",
                     "Engine:", engine == FS ? "FS" : "QuarkStore", "Data directory:", ddir.c_str(),
                     "Working directory:", wdir.c_str(), "Mode:", mode == COMPACT ? "Compact" : "Load",
                     "Levels:", n_levels, "Level multiplier:", level_size_multiplier, "Key size:", key_size,
                     "Value size:", value_size(), "Level 0 max size:", level0_max_size,
                     "SST file size:", sst_file_size >> 20);
        return std::string(buffer);
    }
};