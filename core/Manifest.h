#pragma once

#include <atomic>
#include <cstring>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/config.h"

class Manifest {
   public:
    Manifest(Config* config);
    virtual ~Manifest() = default;

    /**
     * Open the manifest and parse all live SST files.
     */
    virtual int Open() = 0;

    /**
     * Persist the new version of the manifest to storage.
     */
    virtual int Persist() = 0;

    virtual std::string ToString() = 0;

    /**
     * Get the identifiers of the SST files sorted by level.
     */
    std::vector<std::vector<int>> GetSSTs();

    /**
     * Add SST file with identifier id to level level.
     * @param level Level to add SST to.
     * @param id ID of SST file to add.
     */
    void AddToLevel(int level, int id);

    /**
     * Get all identifiers of SST files.
     */
    std::vector<int> FlattenSSTs();

   protected:
    virtual int openCurrent() = 0;

    Config* config_;

    int handler_;
    int current_;

    std::atomic<uint32_t> ctr_;
    std::vector<std::vector<int>> sst_files_;

    uint32_t getNumberOfLiveSSTs();
    std::string getFullPath(std::string name);

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
    void deserializeLevels(char* buf, size_t bufsize);
    void serializeLevels(char* buf);
};