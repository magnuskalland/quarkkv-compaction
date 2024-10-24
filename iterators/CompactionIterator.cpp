#include "../include/CompactionIterator.h"

CompactionIterator::CompactionIterator(
    Config* config, std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>* ssts)
    : Iterator(config)
{
    assert(ssts->size() == config_->n_levels);

    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>::iterator it;
    for (it = ssts->begin(); it != ssts->end(); it++) {
        if ((*it).size() == 0) {
            continue;
        }
        if (it == ssts->begin()) {
            addLevel0SST(*it);
        }
        else {
            addLevelNSST(*it);
        }
    }
}

void CompactionIterator::Next()
{
    // exhausted
    if (heap_.empty()) {
        ptr_ = nullptr;
        return;
    }

    KVPair* prev = ptr_;
    std::shared_ptr<Iterator> it = heap_.top();
    heap_.pop();
    it->Next();

    if (it->Get() != it->End()) {
        heap_.push(it);
    }

    if (heap_.empty()) {
        ptr_ = nullptr;
        return;
    }

    ptr_ = heap_.top()->Get();
    assert(*ptr_ > *prev);
    iterations_++;
}

int CompactionIterator::SeekToFirst()
{
    ptr_ = heap_.top()->Get();
    return 0;
}

void CompactionIterator::addLevel0SST(
    std::set<std::shared_ptr<SST>, SST::SSTComparator> vec)
{
    int ok;
    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = vec.begin(); it != vec.end(); it++) {
        if (!(*it).get()->IsMarkedForCompaction()) {
            continue;
        }
        std::shared_ptr<KVIterator> kvIterator =
            std::make_shared<KVIterator>(config_, *it);
        // seek to first before insertion to heap to enable comparison
        ok = kvIterator.get()->SeekToFirst();
        assert(ok != -1);
        heap_.push(kvIterator);
    }
}

void CompactionIterator::addLevelNSST(
    std::set<std::shared_ptr<SST>, SST::SSTComparator> vec)
{
    if (vec.size() == 0 || !vec.begin()->get()->IsMarkedForCompaction()) {
        return;
    }

    int ok;
    std::shared_ptr<LevelIterator> it = std::make_shared<LevelIterator>(config_, vec);
    // seek to first before insertion to heap to enable comparison
    ok = it.get()->SeekToFirst();
    assert(ok != -1);
    heap_.push(it);
}