#pragma once

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "SSTManager.h"
#include "config.h"

class Manifest {
   public:
    Manifest(Config* config) : config_(config), ctr_(0)
    {
        for (uint32_t level = 0; level < config_->n_levels; level++) {
            sst_files_.emplace_back(std::vector<int>());
        }
    };
    virtual ~Manifest() {}

    /**
     * Open the manifest and parse all live SST files.
     */
    virtual int Open() = 0;

    /**
     * Persist the new version of the manifest to storage.
     */
    virtual int Persist() = 0;

    /**
     * Get IDs of SST files of all levels.
     */
    std::vector<std::vector<int>> GetLevels()
    {
        return sst_files_;
    }

    /**
     * Add ID to level.
     */
    void AddToLevel(int level, int id)
    {
        sst_files_.at(level).emplace_back(id);
    }

   protected:
    Config* config_;

    int handler_;
    int current_;

    std::atomic<uint32_t> ctr_;
    std::vector<std::vector<int>> sst_files_;

    /**
     * Data format:
     * int32 number of levels
     * int32 size of level 0
     * int32 name of first sstfile on level 0
     * ...
     * int32 name of last sstfile on level 0
     * int32 size of level 1
     * ...
     */
    void deserializeLevels(char* buf, size_t bufsize)
    {
        // uint32_t offset = 0;
        // uint32_t level_size = (uint32_t)buf[0];
        // for (uint32_t i = 0; i < bufsize; i += level_size * sizeof(uint32_t)) {
        //     sst_files_.emplace_back(std::vector<int>());
        //     offset += sizeof(uint32_t);
        //     for (uint32_t j = 0; j < level_size; j++) {
        //         sst_files_.at(j).emplace_back((uint32_t)buf[offset]);
        //         offset += sizeof(uint32_t);
        //     }
        //     level_size = (uint32_t)buf[offset];
        // }
    }

    void serializeLevels(char* buf)
    {
        uint32_t offset = 0;
        std::memset(buf, 0, BLOCK_SIZE);

        // add number of levels: 4 bytes
        ssize_t n_levels = sst_files_.size();
        std::memcpy(&buf[offset], (void*)&n_levels, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        // add number of levels on level i: 4 bytes each
        for (uint32_t i = 0; i < sst_files_.size(); i++) {
            uint32_t number = sst_files_.at(i).size();
            std::memcpy(&buf[offset], (void*)&number, sizeof(uint32_t));
            offset = offset + sizeof(uint32_t);

            // add all IDs on level i: 4 bytes each
            for (uint32_t j = 0; j < sst_files_.at(i).size(); j++) {
                uint32_t number = sst_files_.at(i).at(j);
                std::memcpy(&buf[offset], (void*)&number, sizeof(uint32_t));
                offset = offset + sizeof(uint32_t);
            }
        }

        assert(offset == sizeof(uint32_t) + n_levels * sizeof(uint32_t) + getNumberOfLiveSSTs() * sizeof(uint32_t));
    }

    uint32_t getNumberOfLiveSSTs()
    {
        uint32_t count = 0;
        for (uint32_t i = 0; i < sst_files_.size(); i++) {
            count = count + sst_files_.at(i).size();
        }
        return count;
    }
};