
#include "CompacterFS.h"

CompacterFS::CompacterFS(
    Config* config, SSTManager* manager,
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : Compacter(config, manager, ssts)
{
}

int CompacterFS::doCompaction(
    CompactionIterator* ci, std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact,
    uint32_t destLevel)
{
    int ok;
    KVPair *pair, *prev = nullptr;

    std::shared_ptr<SST> sst;
    sst = manager_->NewEmptySST();

    // do compaction
    int c = 0;
    while (ci->Get() != ci->End()) {
        pair = ci->Get();
        c++;

        if (prev && prev->GetKey() == pair->GetKey()) {
            currentStats.merged++;
            assert(prev->GetTimestamp() < pair->GetTimestamp());
            prev = pair;
            ci->Next();
            continue;
        }

        if (sst.get()->IsFull()) {
            ok = finishSSTFile(sst, destLevel);
            if (ok == -1) {
                return -1;
            }
            sst = manager_->NewEmptySST();
        }

        ok = sst.get()->AddKV(pair);
        if (ok == -1) {
            return -1;
        }

        prev = pair;
        ci->Next();
    }

    ok = finishSSTFile(sst, destLevel);
    if (ok == -1) {
        return -1;
    }

    // remove compacted SST files
    for (auto it = toCompact.begin(); it != toCompact.end(); it++) {
        std::shared_ptr<SST> sst = *it;
        ssts_->at(sst.get()->GetLevel()).erase(sst);
        ok = sst.get()->Remove();
        currentStats.deleted++;
        if (ok == -1) {
            return -1;
        }
    }

    assert(currentStats.newSSTs <= toCompact.size());
    return 0;
}