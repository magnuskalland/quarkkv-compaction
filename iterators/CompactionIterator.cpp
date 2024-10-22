#include "../include/CompactionIterator.h"

CompactionIterator::CompactionIterator(Config* config, std::vector<std::vector<std::shared_ptr<SST>>>* ssts)
    : Iterator(config)
{
    assert(ssts->size() == config_->n_levels);

    std::vector<std::vector<std::shared_ptr<SST>>>::iterator outer;
    for (outer = ssts->begin(); outer != ssts->end(); outer++) {
        if ((*outer).size() == 0) {
            continue;
        }

        if (outer == ssts->begin()) {
            addLevel0SST(*outer);
        }
        else {
            addLevelNSST(*outer);
        }
    }

    Next();
}

void CompactionIterator::Next()
{
    while (!ptr_ || heap_.empty()) {
        ptr_ = (*heap_.top()).Get();
        if (ptr_) {
            break;
        }
        heap_.pop();
    }

    if (heap_.empty()) {
        return;
    }

    Iterator* it = heap_.top();
    heap_.pop();
    it->Next();
    heap_.push(it);
}

void CompactionIterator::addLevel0SST(std::vector<std::shared_ptr<SST>> vec)
{
    std::vector<std::shared_ptr<SST>>::iterator it;
    for (it = vec.begin(); it != vec.end(); it++) {
        if (!(*it).get()->IsMarkedForCompaction()) {
            continue;
        }
        KVIterator kvIterator(config_, *it);
        heap_.push(&kvIterator);
    }
}

void CompactionIterator::addLevelNSST(std::vector<std::shared_ptr<SST>> vec)
{
    if (vec.size() == 0 || !vec.at(0).get()->IsMarkedForCompaction()) {
        return;
    }

    LevelIterator it(config_, vec);
    heap_.push(&it);
}