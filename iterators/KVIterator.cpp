#include "../include/KVIterator.h"

KVIterator::KVIterator(Config* config, std::shared_ptr<SST> sst)
    : Iterator(config), sst_(sst)
{
    sst->GetKVAtIndex(sst->GetEntries() - 1, &end_);
}

void KVIterator::Next()
{
    int ok = -1;
    KVPair* prev = ptr_;
    assert(ptr_);

    if (ptr_->GetKey() == end_->GetKey()) {
        // printf("KVIterator %d exhausted\n", sst_.get()->GetID());
        ptr_ = nullptr;
        return;
    }

    if (!(index_ >= 0 && index_ < sst_.get()->GetEntries())) {
        printf(
            "Failing with index_ %d and entries_ %d\nKey is %s and largest key is %s\n",
            index_, sst_.get()->GetEntries(), ptr_->GetKey().c_str(),
            sst_.get()->GetLargestKey().c_str());
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
    // printf("KVIterator %d seeking to first, key is %s\n", sst_.get()->GetID(),
    //        ptr_->GetKey().c_str());
    return ok;
}