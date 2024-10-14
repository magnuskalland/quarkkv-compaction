#pragma once

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "BackgroundWorker.h"
#include "SSTFileManager.h"
#include "config.h"

class Manifest {
   public:
    Manifest(Config* config) : config_(config), manager_(nullptr)
    {
        for (uint32_t level = 0; level < config_->n_levels; level++) sst_files_.emplace_back(std::vector<int>());
    };
    virtual ~Manifest() {}
    virtual int Open() = 0;
    virtual int Populate(uint32_t n) = 0;

   protected:
    virtual int persist() = 0;
    virtual int createManifest(int id) = 0;
    Config* config_;
    SSTFileManager* manager_;
    BackgroundWorker worker_;

    int handler_;
    int current_;
    std::atomic<uint32_t> manifest_ctr_;
    std::atomic<uint32_t> sst_ctr_;
    std::vector<std::vector<int>> sst_files_;
    ssize_t maxManifestSize_ = 1024;
    std::mutex mutex_;

    struct DiskManifest {
        std::vector<uint32_t> ssts;
    };

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
    void parse(char* buf, size_t bufsize)
    {
        uint32_t offset = 0;
        uint32_t level_size = (uint32_t)buf[0];
        for (uint32_t i = 0; i < bufsize; i += level_size * sizeof(uint32_t)) {
            sst_files_.emplace_back(std::vector<int>());
            offset += sizeof(uint32_t);
            for (uint32_t j = 0; j < level_size; j++) {
                sst_files_.at(j).emplace_back((uint32_t)buf[offset]);
                offset += sizeof(uint32_t);
            }
            level_size = (uint32_t)buf[offset];
        }
    }

    uint32_t getLevels()
    {
        uint32_t count = 0;
        for (uint32_t i = 0; i < sst_files_.size(); i++) {
            count = count + sst_files_.at(i).size();
        }
        return count;
    }

    void serializeLevels(char* buf)
    {
        uint32_t offset = 0;
        std::memset(buf, 0, BLOCK_SIZE);
        ssize_t manifest_size = sizeof(uint32_t) + getLevels() * sizeof(uint32_t);
        assert(manifest_size < BLOCK_SIZE);
        std::memcpy(&buf[offset], &manifest_size, sizeof(uint32_t));
        for (uint32_t i = 0; i < sst_files_.size(); i++) {
            uint32_t number = sst_files_.at(i).size();
            std::memcpy(&buf[offset], &number, sizeof(uint32_t));
            offset = offset + sizeof(uint32_t);
            printf("Level %u has %u elements\n", i, number);
            for (uint32_t j = 0; j < sst_files_.at(i).size(); j++) {
                uint32_t number = sst_files_.at(i).at(j);
                std::memcpy(&buf[offset], &number, sizeof(uint32_t));
                offset = offset + sizeof(uint32_t);
            }
        }
    }
};