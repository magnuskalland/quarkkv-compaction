#pragma once
#include <set>

#include "../include/Iterator.h"
#include "../include/KVIterator.h"

class LevelIterator : public Iterator {
   public:
    LevelIterator(Config* config,
                  std::set<std::shared_ptr<SST>, SST::SSTComparator> ssts);
    void Next() override;
    int SeekToFirst() override;

   private:
    std::vector<KVIterator> iterators_;
    uint32_t level_;
    uint32_t index_ = 0;
};