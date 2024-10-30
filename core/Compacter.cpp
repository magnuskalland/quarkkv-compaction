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
    bool verified;
    for (uint32_t level = 0; level < ssts_->size() - 1; level++) {
        std::set<std::shared_ptr<SST>, SST::SSTComparator>*l1 = &ssts_->at(level),
                                       *l2 = &ssts_->at(level + 1);

        // stop compaction if level is smaller than the fanout
        if (l1->size() <= config_->maxSizeOfLevel(level)) {
            break;
        }

        printf("Compacting level %d (%ld SSTs) into level %d (%ld SSTs)\n", level,
               ssts_->at(level).size(), level + 1, ssts_->at(level + 1).size());

        markUpperLevelForCompaction(level);

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
        std::copy_if(l1->begin(), l1->end(), std::inserter(set, set.end()),
                     [](const std::shared_ptr<SST>& sst) {
                         return sst.get()->IsMarkedForCompaction();
                     });
        std::copy_if(l2->begin(), l2->end(), std::inserter(set, set.end()),
                     [](const std::shared_ptr<SST>& sst) {
                         return sst.get()->IsMarkedForCompaction();
                     });

        ok = doCompaction(&ci, set, level + 1);
        if (ok == -1) {
            fprintf(stderr, "error: compaction failed\n");
            return -1;
        }

        verified = verify(level + 1);
        if (!verified) {
            fprintf(stderr, "Compaction on level %d not verified\n", level + 1);
            return -1;
        }
        currentStats.compactions++;
        totalStats.Add(currentStats);
        currentStats.Clear();
    }

    printf("Total compaction stats:\n%s\n", totalStats.ToString().c_str());

    return 0;
}

void Compacter::markUpperLevelForCompaction(uint32_t level)
{
    auto ssts = ssts_->at(level);
    // compact all SSTs
    if (config_->cp == ALL) {
        markLevelForCompaction(level);
        return;
    }

    assert(config_->cp == ONE);

    if (level > 0) {
        // pick random key range to compact (fairness)
        std::srand(std::time(nullptr));
        int index = std::rand() % ssts.size();
        auto it = ssts.begin();
        std::advance(it, index);
        (*it)->MarkForCompaction();
        return;
    }

    assert(level == 0);
    SST* sst = nullptr;
    std::chrono::system_clock::time_point oldest = std::chrono::system_clock::now();
    for (auto it = ssts.begin(); it != ssts.end(); it++) {
        assert(it->get()->IsPersisted());
        if (it->get()->GetPersistTime() < oldest) {
            sst = it->get();
            oldest = it->get()->GetPersistTime();
        }
    }
    assert(sst);
    sst->MarkForCompaction();

    printf("Compacting level 0 SST %d into level 1\n", sst->GetID());
}

void Compacter::markLevelForCompaction(uint32_t level)
{
    for (auto it = ssts_->at(level).begin(); it != ssts_->at(level).end(); it++) {
        (*it).get()->MarkForCompaction();
    }
}

int Compacter::finishSSTFile(std::shared_ptr<SST> sst, uint32_t level)
{
    int ok;
    ok = sst.get()->Persist();
    if (ok == -1) {
        return -1;
    }
    ssts_->at(level).insert(sst);
    sst.get()->SetLevel(level);
    currentStats.newSSTs++;
    return 0;
}

int Compacter::removeCompacted(
    std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact)
{
    int ok;
    for (auto it = toCompact.begin(); it != toCompact.end(); it++) {
        auto sst = *it;
        ssts_->at(sst.get()->GetLevel()).erase(sst);
        ok = sst.get()->Remove();
        currentStats.deleted++;
        if (ok == -1) {
            return -1;
        }
    }
    return 0;
}

void Compacter::initEmptyLevel(uint32_t level)
{
    assert(ssts_->at(level).size() == 0);
    std::set<std::shared_ptr<SST>, SST::SSTComparator> set;
    for (auto it = ssts_->at(level - 1).begin(); it != ssts_->at(level - 1).end(); it++) {
        if ((*it).get()->IsMarkedForCompaction()) {
            (*it).get()->SetLevel(level);
            (*it).get()->UnmarkForCompaction();
            ssts_->at(level).insert(*it);
            set.insert(*it);
        }
    }

    for (auto it = set.begin(); it != set.end(); it++) {
        ssts_->at(level - 1).erase(*it);
    }
}

bool Compacter::verify(uint32_t level)
{
    std::string smallest, largest;
    smallest = ssts_->at(level).begin()->get()->GetSmallestKey();
    for (auto it = ssts_->at(level).begin(); it != std::prev(ssts_->at(level).end());
         it++) {
        largest = (*it).get()->GetLargestKey();
        if (!(smallest < largest)) {
            printf("Error in %s: Smallest key larger than largest key\n%s\n%s\n",
                   (*it).get()->GetName().c_str(), smallest.c_str(), largest.c_str());
            return false;
        }
        smallest = std::next(it)->get()->GetSmallestKey();
        if (!(largest < smallest)) {
            printf("Largest key in %s larger than smallest key in %s\n%s\n%s\n",
                   (*it).get()->GetName().c_str(),
                   std::next(it)->get()->GetName().c_str(), largest.c_str(),
                   smallest.c_str());
            return false;
        }
    }
    return true;
}
