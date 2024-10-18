#pragma once

#include <atomic>
#include <cstring>
#include <mutex>
#include <sstream>
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
    std::vector<std::vector<int>> GetSSTs()
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

    std::vector<int> FlattenSSTs()
    {
        return std::accumulate(sst_files_.begin(), sst_files_.end(), std::vector<int>(),
                               [](std::vector<int>& acc, const std::vector<int>& vec) {
                                   acc.insert(acc.end(), vec.begin(), vec.end());
                                   return acc;
                               });
    }

    std::string ToString()
    {
        std::ostringstream oss;

        for (uint32_t i = 0; i < sst_files_.size(); i++) {
            oss << "Level " << i << ": ";
            for (uint32_t j = 0; j < sst_files_.at(i).size(); j++) {
                oss << sst_files_.at(i).at(j) << " ";
            }
            oss << "\n";
        }

        return oss.str();
    }

   protected:
    Config* config_;

    int handler_;
    int current_;

    std::atomic<uint32_t> ctr_;
    std::vector<std::vector<int>> sst_files_;

    /**
     * Data format:
     * uint32_t number of levels
     * uint32_t number of entries on level 0
     * uint32_t name of first sstfile on level 0
     * ...
     * uint32_t name of last sstfile on level 0
     * uint32_t number of entries on level 1
     * ...
     */
    void deserializeLevels(char* buf, size_t bufsize)
    {
        uint32_t offset = 0;
        uint32_t levels = (uint32_t)buf[offset];
        offset += sizeof(uint32_t);

        for (uint32_t level = 0; level < levels; level++) {
            uint32_t entries = (uint32_t)buf[offset];
            offset += sizeof(uint32_t);

            for (uint32_t j = 0; j < entries; j++) {
                uint32_t id = (uint32_t)buf[offset];
                sst_files_.at(level).emplace_back(id);
                offset += sizeof(uint32_t);
            }
        }
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
        for (uint32_t level = 0; level < sst_files_.size(); level++) {
            uint32_t number = sst_files_.at(level).size();
            std::memcpy(&buf[offset], (void*)&number, sizeof(uint32_t));
            offset = offset + sizeof(uint32_t);

            // add all IDs on level i: 4 bytes each
            for (uint32_t entry = 0; entry < sst_files_.at(level).size(); entry++) {
                uint32_t number = sst_files_.at(level).at(entry);
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