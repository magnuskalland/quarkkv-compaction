#include "../include/Compacter.h"

#include <math.h>

#include "../include/CompactionIterator.h"

Compacter::Compacter(
    Config* config, std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : config_(config), ssts_(ssts)
{
}

int Compacter::Compact()
{
    assert(ssts_->size() == config_->n_levels);
    assert(ssts_->at(0).size() > 1);

    int ok;
    for (uint32_t level = 0; level < ssts_->size() - 1; level++) {
        printf("\nStarting compaction iteration %d\n", level);
        std::set<std::shared_ptr<SST>, SST::SSTComparator> l = ssts_->at(level);
        if (ssts_->at(level).size() <= pow(config_->fanout, level)) {
            break;
        }

        markLevelForCompaction(level);
        markLevelForCompaction(level + 1);
        CompactionIterator ci(config_, ssts_);
        printf("Compacting level %d with level %d\n", level, level + 1);

        std::set<std::shared_ptr<SST>, SST::SSTComparator> set;
        set.insert(ssts_->at(level).begin(), ssts_->at(level).end());
        set.insert(ssts_->at(level + 1).begin(), ssts_->at(level + 1).end());
        ok = doCompaction(&ci, set, level + 1);
        if (ok == -1) {
            fprintf(stderr, "error: compaction failed\n");
            return -1;
        }
    }

    return 0;
}

void Compacter::markLevelForCompaction(uint32_t level)
{
    printf("Marking level %d for compaction (%ld SSTs)\n", level,
           ssts_->at(level).size());
    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = ssts_->at(level).begin(); it != ssts_->at(level).end(); it++) {
        (*it).get()->MarkForCompaction();
    }
}