#include "../include/LevelIterator.h"

#include "../include/KVIterator.h"

LevelIterator::LevelIterator(Config* config,
                             std::set<std::shared_ptr<SST>, SST::SSTComparator> ssts)
    : Iterator(config)
{
    assert(ssts.size() > 0);
    level_ = ssts.begin()->get()->GetLevel();

    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = ssts.begin(); it != ssts.end(); it++) {
        KVIterator kvIterator(config_, *it);
        iterators.emplace_back(kvIterator);
    }

    ptr_ = iterators.at(0).Get();
}

void LevelIterator::Next()
{
    printf("Calling LevelIterator::Next()\n");

    // init
    if (!ptr_ && (int)index == -1) {
        index = index + 1;
    }

    KVPair* prev = ptr_;
    while (index < iterators.size()) {
        KVIterator iter = iterators.at(index);

        // exhausted current SST
        if (iter.Get() == iter.End()) {
            index = index + 1;
            continue;
        }

        ptr_ = iter.Get();
        iter.Next();
        assert((!ptr_ && !iter.Get()) || (ptr_->GetKey() > prev->GetKey()));
        return;
    }

    // iterator exhausted
    ptr_ = nullptr;
}