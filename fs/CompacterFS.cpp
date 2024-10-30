
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

    int c = 0;

    // do compaction
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

        currentStats.writeAmplificationKVs += 1;
        currentStats.writeAmplificationBytes += config_->kv_size();

        prev = pair;
        ci->Next();
    }

    printf("Merged %d KVs\n", c);

    ok = finishSSTFile(sst, destLevel);
    if (ok == -1) {
        return -1;
    }

    ok = removeCompacted(toCompact);
    if (ok == -1) {
        return -1;
    }

    if (!(currentStats.newSSTs <= toCompact.size())) {
        printf("New SSTs: %d, SSTs to compact: %ld\n", currentStats.newSSTs,
               toCompact.size());
    }

    assert(currentStats.newSSTs <= toCompact.size());
    return 0;
}