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
        iterators_.emplace_back(kvIterator);
    }
}

void LevelIterator::Next()
{
    assert(index_ >= 0 && index_ < iterators_.size());

    int ok;
    KVPair *prev = ptr_, *next;

    KVIterator* iter = &iterators_.at(index_);
    iter->Next();

    // exhausted current SST
    if (iter->Get() == iter->End()) {
        index_ = index_ + 1;
        // iterator exhausted
        if (!(index_ < iterators_.size())) {
            ptr_ = nullptr;
            return;
        }
        iter = &iterators_.at(index_);
        ok = iter->SeekToFirst();
        assert(ok != -1);
    }

    ptr_ = iter->Get();
    if (!(!ptr_ || *ptr_ > *prev)) {
        if (!(ptr_)) {
            printf("ptr_ is null\n");
        }
        if (!(*ptr_ > *prev)) {
            printf(
                "(index %d, iteration %d) ptr_ is not greater than previous:\n%s\n%s\n",
                index_, iterations_, ptr_->ToString().c_str(), ptr_->ToString().c_str());
        }
    }
    assert(!ptr_ || *ptr_ > *prev);
    iterations_++;
}

int LevelIterator::SeekToFirst()
{
    assert(index_ == 0);
    int ok;
    ok = iterators_.at(0).SeekToFirst();
    if (ok == -1) {
        return -1;
    }
    ptr_ = iterators_.at(0).Get();
    iterations_++;
    return 0;
}