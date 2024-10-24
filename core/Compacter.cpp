#include "../include/Compacter.h"

#include <math.h>

#include "../include/CompactionIterator.h"

Compacter::Compacter(
    Config* config, SSTManager* manager,
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : config_(config), manager_(manager), ssts_(ssts)
{
}

int Compacter::Compact()
{
    assert(ssts_->size() == config_->n_levels);
    assert(ssts_->at(0).size() > 1);

    int ok;
    for (uint32_t level = 0; level < ssts_->size() - 1; level++) {
        std::set<std::shared_ptr<SST>, SST::SSTComparator> l = ssts_->at(level);
        if (ssts_->at(level).size() <= pow(config_->fanout, level)) {
            break;
        }

        printf("Compacting %ld SSTs from level %d into %ld SSTs on level %d\n",
               ssts_->at(level).size(), level, ssts_->at(level + 1).size(), level + 1);
        markLevelForCompaction(level);
        markLevelForCompaction(level + 1);
        CompactionIterator ci(config_, ssts_);

        std::set<std::shared_ptr<SST>, SST::SSTComparator> set;
        set.insert(ssts_->at(level).begin(), ssts_->at(level).end());
        set.insert(ssts_->at(level + 1).begin(), ssts_->at(level + 1).end());
        ok = doCompaction(&ci, set, level + 1);
        if (ok == -1) {
            fprintf(stderr, "error: compaction failed\n");
            return -1;
        }
    }

    currentStats.compactions++;
    totalStats.Add(currentStats);

    printf("Current compaction stats:\n%s\n", currentStats.ToString().c_str());
    printf("Total compaction stats:\n%s\n", totalStats.ToString().c_str());

    return 0;
}

void Compacter::markLevelForCompaction(uint32_t level)
{
    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = ssts_->at(level).begin(); it != ssts_->at(level).end(); it++) {
        (*it).get()->MarkForCompaction();
    }
}

int Compacter::finishSSTFile(std::shared_ptr<SST> sst, uint32_t level)
{
    printf("Finishing %s (%d entries)\n", sst.get()->GetName().c_str(),
           sst.get()->GetEntries());
    int ok;
    ssts_->at(level).insert(sst);
    sst.get()->SetLevel(level);
    ok = sst.get()->Persist();
    if (ok == -1) {
        return -1;
    }
    currentStats.newSSTs++;
    return 0;
}
