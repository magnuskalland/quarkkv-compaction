#pragma once

#include "../include/Compacter.h"
#include "../include/CompactionIterator.h"

class CompacterFS : public Compacter {
   public:
    CompacterFS(Config* config,
                std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts);

   protected:
    int doCompaction(CompactionIterator* ci,
                     std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact,
                     uint32_t destLevel) override;
};