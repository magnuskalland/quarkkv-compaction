#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../iterator/InternalIterator.h"
#include "../utils/KeyGenerator.h"
#include "SST.h"
#include "config.h"

class SSTManager {
   public:
    SSTManager(Config* config) : config_(config), keygen_(new UniformKeyGenerator(config->key_size)) {}
    virtual ~SSTManager() = default;

    /**
     * Create new empty SST.
     * @return A pointer to the newly created SST.
     */
    virtual std::shared_ptr<SST> NewEmptySST() = 0;

    /**
     * Populate an existing SST.
     */
    virtual int PopulateSST(SST& sst) = 0;

    /**
     * Open and read SST file and assign it to level a level.
     * @param id of SST file.
     * @return A pointer to the read and parsed SST file.
     */
    virtual std::shared_ptr<SST> ReadSST(uint32_t id) = 0;

    /**
     * @param key The key to look up.
     * @param dest Destination for the value.
     * @return The newest value associated with the key, into dest, or nullptr if
     * the key does not exist.
     */
    virtual int Get(SST* sst, std::string key, KVPair** dest) = 0;

    /**
     * Creates an iterator for this SST file.
     */
    // virtual InternalIterator CreateIterator() = 0;

   protected:
    Config* config_;
    KeyGenerator* keygen_;
};