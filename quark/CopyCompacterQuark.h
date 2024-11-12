#pragma once

#include "../include/Compacter.h"

class CopyCompacterQuark : public Compacter {
   public:
    CopyCompacterQuark(Config* config, SSTManager* manager,
                   std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts);

   protected:
    int merge(std::shared_ptr<SST> sst, KVPair* pair) override;
};