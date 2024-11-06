#pragma once

#include "../include/Compacter.h"
#include "../include/CompactionIterator.h"

class CompacterFS : public Compacter {
   public:
    CompacterFS(Config* config, SSTManager* manager,
                std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts);

   protected:
    int merge(std::shared_ptr<SST> sst, KVPair* pair) override;
};