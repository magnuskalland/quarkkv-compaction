#include "CopyCompacterQuark.h"

#include "SSTQuark.h"

CopyCompacterQuark::CopyCompacterQuark(
    Config* config, SSTManager* manager,
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : Compacter(config, manager, ssts)
{
}

int CopyCompacterQuark::merge(std::shared_ptr<SST> sst, KVPair* pair)
{
    int ok = sst.get()->AddKV(pair);
    if (ok == -1) {
        return -1;
    }

    currentStats.writeAmplificationKVs += 1;
    currentStats.writeAmplificationBytes += config_->kv_size();
    return 0;
}
