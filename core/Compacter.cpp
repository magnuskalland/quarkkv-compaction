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

    currentStats.Clear();

    int ok;
    for (uint32_t level = 0; level < ssts_->size() - 1; level++) {
        std::set<std::shared_ptr<SST>, SST::SSTComparator>*l1 = &ssts_->at(level),
                                       *l2 = &ssts_->at(level + 1);

        // stop compaction if level is smaller than the fanout
        if (l1->size() <= pow(config_->fanout, level)) {
            break;
        }

        markLevelForCompaction(level);

        // if next level is empty, simply move SSTs
        if (level != 0 && l2->size() == 0) {
            initEmptyLevel(level + 1);
            break;
        }

        // create compaction iterator for this and the next level
        markLevelForCompaction(level + 1);
        CompactionIterator ci(config_, ssts_);
        ok = ci.SeekToFirst();
        if (ok == -1) {
            return -1;
        }

        // do compaction
        std::set<std::shared_ptr<SST>, SST::SSTComparator> set;
        set.insert(l1->begin(), l1->end());
        set.insert(l2->begin(), l2->end());
        ok = doCompaction(&ci, set, level + 1);
        if (ok == -1) {
            fprintf(stderr, "error: compaction failed\n");
            return -1;
        }

        currentStats.compactions++;
        totalStats.Add(currentStats);
        currentStats.Clear();
    }

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

void Compacter::initEmptyLevel(uint32_t level)
{
    assert(ssts_->at(level).size() == 0);
    std::set<std::shared_ptr<SST>, SST::SSTComparator> set;
    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = ssts_->at(level - 1).begin(); it != ssts_->at(level - 1).end(); it++) {
        if ((*it).get()->IsMarkedForCompaction()) {
            (*it).get()->SetLevel(level);
            (*it).get()->UnmarkForCompaction();
            ssts_->at(level).insert(*it);
            set.insert(*it);
        }
    }

    for (it = set.begin(); it != set.end(); it++) {
        ssts_->at(level - 1).erase(*it);
    }
}