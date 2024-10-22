#pragma once

#include "../include/Compacter.h"
#include "../include/CompactionIterator.h"

class CompacterFS : public Compacter {
   public:
    CompacterFS(Config* config, std::vector<std::vector<std::shared_ptr<SST>>>* ssts);

   protected:
    int doCompaction(CompactionIterator* ci, std::set<std::shared_ptr<SST>> toCompact, uint32_t destLevel) override;
};