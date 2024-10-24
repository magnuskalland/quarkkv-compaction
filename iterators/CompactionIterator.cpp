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
    ptr_ = heap_.top()->Get();
}

void CompactionIterator::Next()
{
    // exhausted
    if (heap_.empty()) {
        ptr_ = nullptr;
        return;
    }

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
}

void CompactionIterator::addLevel0SST(
    std::set<std::shared_ptr<SST>, SST::SSTComparator> vec)
{
    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = vec.begin(); it != vec.end(); it++) {
        if (!(*it).get()->IsMarkedForCompaction()) {
            continue;
        }
        std::shared_ptr<KVIterator> kvIterator =
            std::make_shared<KVIterator>(config_, *it);
        heap_.push(kvIterator);
    }
}

void CompactionIterator::addLevelNSST(
    std::set<std::shared_ptr<SST>, SST::SSTComparator> vec)
{
    if (vec.size() == 0 || !vec.begin()->get()->IsMarkedForCompaction()) {
        return;
    }

    std::shared_ptr<LevelIterator> it = std::make_shared<LevelIterator>(config_, vec);
    heap_.push(it);
}