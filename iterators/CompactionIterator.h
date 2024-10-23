#pragma once

#include <memory>
#include <queue>
#include <set>

#include "../include/KVPair.h"
#include "../include/LevelIterator.h"
#include "../include/SST.h"
#include "../include/config.h"

class CompactionIterator : public Iterator {
   public:
    CompactionIterator(
        Config* config,
        std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts);
    void Next() override;

   private:
    std::priority_queue<std::shared_ptr<Iterator>, std::vector<std::shared_ptr<Iterator>>,
                        std::less<std::shared_ptr<Iterator>>>
        heap_;
    void addLevel0SST(std::set<std::shared_ptr<SST>, SST::SSTComparator> vec);
    void addLevelNSST(std::set<std::shared_ptr<SST>, SST::SSTComparator> vec);
};