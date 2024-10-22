#include "../include/KVIterator.h"

KVIterator::KVIterator(Config* config, std::shared_ptr<SST> sst) : Iterator(config), sst_(sst)
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
        assert(index == 0 || index == sst_.get()->GetEntries());
    }

    if (ptr_->GetKey() == sst_.get()->GetLargestKey()) {
        ptr_ = nullptr;
        return;
    }

    ok = sst_->GetKVAtIndex(index++, &pair);
    assert(ok != -1);

    ptr_ = pair;
    assert(ptr_ > prev);
}