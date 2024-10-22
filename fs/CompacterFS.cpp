
#include "CompacterFS.h"

CompacterFS::CompacterFS(Config* config,
                         std::vector<std::vector<std::shared_ptr<SST>>>* ssts)
    : Compacter(config, ssts)
{
}

int CompacterFS::doCompaction(CompactionIterator* ci,
                              std::set<std::shared_ptr<SST>> toCompact,
                              uint32_t destLevel)
{
    int ok;
    return 0;
}