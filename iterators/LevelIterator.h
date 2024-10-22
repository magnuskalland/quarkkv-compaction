#pragma once
#include <queue>

#include "../include/Iterator.h"
#include "../include/KVIterator.h"

class LevelIterator : public Iterator {
   public:
    LevelIterator(Config* config, std::vector<std::shared_ptr<SST>> ssts);
    void Next() override;

   private:
    std::vector<KVIterator> iterators;
    uint32_t level_;
    uint32_t index = -1;
};