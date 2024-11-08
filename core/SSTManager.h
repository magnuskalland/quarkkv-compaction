#pragma once

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../utils/KeyGenerator.h"
#include "MemTable.h"
#include "SST.h"
#include "config.h"

class SSTManager {
   public:
    SSTManager(Config* config);
    ~SSTManager();

    /**
     * Create new empty SST.
     * @return A pointer to the newly created SST.
     */
    std::shared_ptr<SST> NewEmptySST();

    /**
     * Populate an existing SST.
     */
    int PopulateSST(SST& sst);

    /**
     * Open and read SST file and assign it to level a level.
     * @param id of SST file.
     * @return A pointer to the read and parsed SST file.
     */
    std::shared_ptr<SST> ReadSST(uint32_t id);

    /**
     * @param key The key to look up.
     * @param dest Destination for the value.
     * @return The newest value associated with the key, into dest, or nullptr if
     * the key does not exist.
     */
    int Get(SST* sst, std::string key, KVPair** dest);

    /**
     * @param table Table to flush.
     * @param sst Destination of resulting SST in-memory representation.
     * @return -1 if IO error, otherwise 0.
     */
    int FlushToSST(MemTable* table, std::shared_ptr<SST>& sst);

    /**
     * Deletes the given SST from underlying storage.
     * @param sst SST to delete.
     * @return -1 if error, 0 otherwise.
     */
    int RemoveSST(std::shared_ptr<SST>& sst);

    uint64_t ClockGetAndIncrement();

   private:
    Config* config_;
    KeyGenerator* keygen_;
    std::atomic<int> ctr_;
    uint64_t clock_; /* monotonically increasing, "atomic" */
};