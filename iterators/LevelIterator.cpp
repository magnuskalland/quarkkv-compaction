#include "../include/LevelIterator.h"

#include "../include/KVIterator.h"

LevelIterator::LevelIterator(Config* config, std::vector<std::shared_ptr<SST>> ssts) : Iterator(config)
{
    assert(ssts.size() > 0);
    level_ = ssts.at(0).get()->GetLevel();

    std::vector<std::shared_ptr<SST>>::iterator it;
    for (it = ssts.begin(); it != ssts.end(); it++) {
        KVIterator kvIterator(config_, *it);
        iterators.emplace_back(kvIterator);
    }

    Next();
}

void LevelIterator::Next()
{
    int ok = -1;
    KVPair* prev = ptr_;

    if (!ptr_) {
        index = index + 1;
    }

    if (index >= iterators.size()) {
        ptr_ = nullptr;
        return;
    }

    KVIterator iter = iterators.at(index);
    iter.Next();
    ptr_ = iter.Get();
    assert((!ptr_ && !iter.Get()) || (ptr_ > prev));
}