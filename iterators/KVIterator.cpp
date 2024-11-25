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

int KVIterator::Seek(std::string key)
{
    assert(index_ == 0);
    int ok;
    int index;

    if (sst_.get()->GetSmallestKey() > key) {
        ptr_ = nullptr;
        return 0;
    }

    if (sst_.get()->GetLargestKey() < key) {
        ptr_ = nullptr;
        return 0;
    }

    index = sst_.get()->SearchClosest(key);
    if (index == -1) {
        ptr_ = nullptr;
        return 0;
    }

    ok = sst_->GetKVAtIndex(index, &ptr_);
    index_ = index;
    return ok;
}

int KVIterator::SeekToFirst()
{
    int ok;
    assert(index_ == 0);
    iterations_++;
    ok = sst_->GetKVAtIndex(index_, &ptr_);
    return ok;
}