#pragma once

#include <set>
#include <vector>

#include "../include/SST.h"
#include "../include/config.h"

class Compacter {
   public:
    Compacter(Config* config, std::vector<std::vector<std::shared_ptr<SST>>>* ssts);

    int Compact();

   protected:
    /**
     * @param ssts SST files to compact.
     * @param destLevel Level to compact to.
     * @param dest Vector of SSTs to insert compacted SST files to.
     */
    virtual int doCompaction(CompactionIterator* ci, std::set<std::shared_ptr<SST>> toCompact, uint32_t destLevel) = 0;

   private:
    Config* config_;
    std::vector<std::vector<std::shared_ptr<SST>>>* ssts_;
    void markLevelForCompaction(uint32_t level);
};