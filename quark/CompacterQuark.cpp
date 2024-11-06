#include "CompacterQuark.h"

#include "SSTQuark.h"

CompacterQuark::CompacterQuark(
    Config* config, SSTManager* manager,
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : Compacter(config, manager, ssts)
{
}

int CompacterQuark::merge(std::shared_ptr<SST> sst, KVPair* pair)
{
    return std::static_pointer_cast<SSTQuark>(sst).get()->MoveAndAddKV(pair);
}
