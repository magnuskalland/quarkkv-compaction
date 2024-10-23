
#include "CompacterFS.h"

CompacterFS::CompacterFS(
    Config* config, std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : Compacter(config, ssts)
{
}

int CompacterFS::doCompaction(
    CompactionIterator* ci, std::set<std::shared_ptr<SST>, SST::SSTComparator> toCompact,
    uint32_t destLevel)
{
    int ok;
    KVPair *pair, *prev = nullptr;

    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = toCompact.begin(); it != toCompact.end(); it++) {
        std::shared_ptr<SST> sst = *it;
        ssts_->at(sst.get()->GetLevel()).erase(sst);
        ssts_->at(destLevel).insert(sst);
        sst.get()->SetLevel(destLevel);
    }

    while (ci->Get() != ci->End()) {
        pair = ci->Get();

        if (prev && prev->GetKey() == pair->GetKey()) {
            assert(prev->GetTimestamp() == pair->GetTimestamp());
            printf("Skipping because of same keys\n");
            continue;
        }

        prev = pair;
        ci->Next();
    }

    return 0;
}