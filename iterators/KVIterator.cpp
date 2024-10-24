#include "../include/KVIterator.h"

KVIterator::KVIterator(Config* config, std::shared_ptr<SST> sst)
    : Iterator(config), sst_(sst)
{
    sst->GetKVAtIndex(sst->GetEntries() - 1, &end_);
    Next();
}

void KVIterator::Next()
{
    int ok = -1;
    KVPair* prev = ptr_;
    KVPair* pair;

    if (!ptr_) {
        assert(index_ == 0 || index_ == sst_.get()->GetEntries());
    }

    if (ptr_ && ptr_->GetKey() == sst_.get()->GetLargestKey()) {
        ptr_ = nullptr;
        return;
    }

    ok = sst_->GetKVAtIndex(index_, &pair);
    assert(ok != -1);
    index_ = index_ + 1;

    ptr_ = pair;
    assert(!prev || *ptr_ > *prev);
}