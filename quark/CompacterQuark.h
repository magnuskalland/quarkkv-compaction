#pragma once

#include "../include/Compacter.h"

class CompacterQuark : public Compacter {
   public:
    CompacterQuark(Config* config, SSTManager* manager,
                   std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts);

   protected:
    int doCompaction(CompactionIterator* ci,
                     std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact,
                     uint32_t destLevel) override;
};