#pragma once

#include <memory>
#include <queue>

#include "../include/KVPair.h"
#include "../include/LevelIterator.h"
#include "../include/SST.h"
#include "../include/config.h"

class CompactionIterator : public Iterator {
   public:
    CompactionIterator(Config* config, std::vector<std::vector<std::shared_ptr<SST>>>* ssts);
    void Next() override;

   private:
    std::priority_queue<Iterator*, std::vector<Iterator*>, std::greater<Iterator*>> heap_;
    void addLevel0SST(std::vector<std::shared_ptr<SST>> vec);
    void addLevelNSST(std::vector<std::shared_ptr<SST>> vec);
};