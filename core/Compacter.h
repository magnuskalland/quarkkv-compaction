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
        uint32_t duplicate = 0;
        uint32_t deleted = 0;
        uint32_t writeAmplificationKVs = 0;
        uint64_t writeAmplificationBytes = 0;

        void Add(struct CompactionStats other)
        {
            compactions += other.compactions;
            newSSTs += other.newSSTs;
            duplicate += other.duplicate;
            deleted += other.deleted;
            writeAmplificationKVs += other.writeAmplificationKVs;
            writeAmplificationBytes += other.writeAmplificationBytes;
        }
        void Clear()
        {
            compactions = 0;
            newSSTs = 0;
            duplicate = 0;
            deleted = 0;
            writeAmplificationKVs = 0;
            writeAmplificationBytes = 0;
        }
        std::string ToString()
        {
            std::ostringstream oss;
            oss << "Compactions: " << compactions << "\n";
            oss << "New SSTs: " << newSSTs << "\n";
            oss << "Deleted SSTs: " << deleted << "\n";
            oss << "Duplicate keys merged: " << duplicate << "\n";
            oss << "Write amplification: " << writeAmplificationKVs << " KV-pairs, "
                << (writeAmplificationBytes >> 20) << " MiB\n";
            return oss.str();
        }
    } currentStats, totalStats;

   protected:
    /**
     * @param ssts SST files to compact.
     * @param destLevel Level to compact to.
     * @param dest Vector of SSTs to insert compacted SST files to.
     */
    int doCompaction(CompactionIterator* ci,
                     std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact,
                     uint32_t destLevel);

    virtual int merge(std::shared_ptr<SST> sst, KVPair* pair) = 0;

   protected:
    Config* config_;
    SSTManager* manager_;
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts_;
    int finishSSTFile(std::shared_ptr<SST> sst, uint32_t level);
    int removeCompacted(std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact);

   private:
    void markUpperLevelForCompaction(uint32_t level);
    void markLevelForCompaction(uint32_t level);
    void initEmptyLevel(uint32_t level);
    bool verify(uint32_t level);
};