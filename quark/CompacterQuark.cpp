#include "CompacterQuark.h"

#include "SSTQuark.h"

CompacterQuark::CompacterQuark(
    Config* config, SSTManager* manager,
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : Compacter(config, manager, ssts)
{
}

int CompacterQuark::doCompaction(
    CompactionIterator* ci, std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact,
    uint32_t destLevel)
{
    int ok;
    KVPair *pair, *prev = nullptr;

    std::shared_ptr<SST> sst;
    sst = manager_->NewEmptySST();

    while (ci->Get() != ci->End()) {
        pair = ci->Get();

        // skip duplicate old KV-pair
        if (prev && prev->GetKey() == pair->GetKey()) {
            currentStats.merged++;
            assert(prev->GetTimestamp() < pair->GetTimestamp());
            prev = pair;
            ci->Next();
            continue;
        }

        // create new
        if (sst.get()->IsFull()) {
            ok = finishSSTFile(sst, destLevel);
            if (ok == -1) {
                return -1;
            }
            sst = manager_->NewEmptySST();
        }

        ok = std::static_pointer_cast<SSTQuark>(sst).get()->MoveAndAddKV(pair);
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

    ok = removeCompacted(toCompact);
    if (ok == -1) {
        return -1;
    }

    assert(currentStats.newSSTs <= toCompact.size());
    return 0;
}