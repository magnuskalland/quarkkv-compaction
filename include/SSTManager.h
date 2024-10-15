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
    virtual ~SSTManager() {}

    /**
     * Create new empty SST.
     */
    virtual std::shared_ptr<SST> NewEmptySST() = 0;

    /**
     * Populate an existing SST.
     */
    virtual int PopulateSST(SST& sst) = 0;

    /**
     * Open and read SST file and assign it to level a level.
     * @param id of SST file.
     * @param level Level to assign SST file to.
     */
    virtual int ReadSST(uint32_t id, uint32_t level) = 0;

    /**
     * Creates an iterator for this SST file.
     */
    // virtual InternalIterator CreateIterator() = 0;

   protected:
    Config* config_;
    KeyGenerator* keygen_;
};