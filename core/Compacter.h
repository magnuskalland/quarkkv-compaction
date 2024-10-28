#pragma once

#include <set>
#include <vector>

#include "../include/CompactionIterator.h"
#include "../include/SST.h"
#include "../include/SSTManager.h"
#include "../include/config.h"

class Compacter {
   public:
    Compacter(Config* config, SSTManager* manager,
              std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts);
    virtual ~Compacter() = default;

    int Compact();
    struct CompactionStats {
        uint32_t compactions = 0;
        uint32_t newSSTs = 0;
        uint32_t merged = 0;
        uint32_t deleted = 0;
        void Add(struct CompactionStats other)
        {
            compactions += other.compactions;
            newSSTs += other.newSSTs;
            merged += other.merged;
            deleted += other.deleted;
        }
        void Clear()
        {
            compactions = 0;
            newSSTs = 0;
            merged = 0;
            deleted = 0;
        }
        std::string ToString()
        {
            std::ostringstream oss;
            oss << "Compactions: " << compactions << "\n";
            oss << "New SSTs: " << newSSTs << "\n";
            oss << "Deleted SSTs: " << deleted << "\n";
            oss << "Merged keys: " << merged;
            return oss.str();
        }
    } currentStats, totalStats;

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
    SSTManager* manager_;
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts_;
    int finishSSTFile(std::shared_ptr<SST> sst, uint32_t level);

   private:
    void markUpperLevelForCompaction(uint32_t level);
    void markLevelForCompaction(uint32_t level);
    void initEmptyLevel(uint32_t level);
    bool verify(uint32_t level);
};