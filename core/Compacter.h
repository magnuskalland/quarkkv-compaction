#pragma once

#include <set>
#include <vector>

#include "../include/CompactionIterator.h"
#include "../include/SST.h"
#include "../include/config.h"

class Compacter {
   public:
    Compacter(Config* config,
              std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts);

    int Compact();

   protected:
    /**
     * @param ssts SST files to compact.
     * @param destLevel Level to compact to.
     * @param dest Vector of SSTs to insert compacted SST files to.
     */
    virtual int doCompaction(CompactionIterator* ci,
                             std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact,
                             uint32_t destLevel) = 0;

   protected:
    Config* config_;
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts_;

   private:
    void markLevelForCompaction(uint32_t level);
};