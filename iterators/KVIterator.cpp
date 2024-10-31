#include "../include/KVIterator.h"

KVIterator::KVIterator(Config* config, std::shared_ptr<SST> sst)
    : Iterator(config), sst_(sst)
{
    sst->GetKVAtIndex(sst->GetEntries() - 1, &end_);
}

void KVIterator::Next()
{
    int ok;
    KVPair* prev = ptr_;
    assert(ptr_);

    if (ptr_->GetKey() == end_->GetKey()) {
        ptr_ = nullptr;
        return;
    }

    ok = sst_->GetKVAtIndex(++index_, &ptr_);
    assert(ok != -1);
    assert(!prev || *ptr_ > *prev);
    iterations_++;
}

int KVIterator::SeekToFirst()
{
    int ok;
    assert(index_ == 0);
    iterations_++;
    ok = sst_->GetKVAtIndex(index_, &ptr_);
    return ok;
}