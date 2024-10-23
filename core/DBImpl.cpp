
#include "../include/DBImpl.h"

#include <cassert>
#include <chrono>
#include <iterator>
#include <thread>

#include "../fs/CompacterFS.h"
#include "../fs/ManifestFS.h"
#include "../include/CompactionIterator.h"

DBImpl::DBImpl(Config* config) : config_(config)
{
    manifest_ = new ManifestFS(config_);
    manager_ = new SSTManager(config_);
    memTable_ = new MemTable(config_);
    compacter_ = new CompacterFS(config_, &ssts_);

    for (uint32_t level = 0; level < config_->n_levels; level++) {
        ssts_.emplace_back(std::set<std::shared_ptr<SST>, SST::SSTComparator>());
    }
};

int DBImpl::Open()
{
    printf("Opening DBImpl\n");
    assert(verifyConfig());

    int ok;
    ok = manifest_->Open();
    if (ok != 0) {
        return -1;
    }

    std::vector<std::vector<int>> ssts = manifest_->GetSSTs();
    for (uint32_t i = 0; i < ssts.size(); i++) {
        std::vector<int>::iterator it;
        std::vector<int> level = ssts.at(i);
        for (it = level.begin(); it != level.end(); it++) {
            std::shared_ptr<SST> sst = manager_->ReadSST(*it);
            if (sst == nullptr) {
                return -1;
            }
            sst.get()->SetLevel(i);
            ssts_.at(i).insert(sst);
        }
    }

    return 0;
}

int DBImpl::Close()
{
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>::iterator outer;
    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (outer = ssts_.begin(); outer != ssts_.end(); ++outer) {
        for (it = outer->begin(); it != outer->end(); ++it) {
            delete (*it).get();
        }
    }
    delete manifest_;
    delete manager_;
    delete memTable_;
    return 0;
}

int DBImpl::Get(std::string key, std::string& dest)
{
    int ok;
    KVPair* kv;

    padKey(key);

    // search in memtable
    kv = memTable_->Get(key);
    if (kv) {
        printf("Found key in memtable\n");
        dest = kv->ToString();
        return 0;
    }

    KVPair* latest = nullptr;
    SST* sst;
    std::set<std::shared_ptr<SST>, SST::SSTComparator> level0 = ssts_.at(0);

    // search in level 0
    std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
    for (it = level0.begin(); it != level0.end(); ++it) {
        sst = (*it).get();
        ok = manager_->Get(sst, key, &kv);
        if (ok == -1) {
            return -1;
        }

        if (kv && (!latest || kv > latest)) {
            latest = kv;
        }
    }

    if (latest) {
        printf("Found KV at level 0\n");
        dest = latest->ToString();
        return 0;
    }

    // search in rest of levels
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>::iterator outer =
        ssts_.begin();
    outer++;
    int level = 1;
    for (; outer != ssts_.end(); ++outer) {
        for (it = outer->begin(); it != outer->end(); ++it) {
            sst = (*it).get();
            ok = manager_->Get(sst, key, &kv);
            if (ok == -1) {
                return -1;
            }

            if (kv) {
                printf("Found KV at level %d\n", level);
                dest = kv->ToString();
                return 0;
            }
        }
        level++;
    }

    printf("Failed to find key\n");
    return 1;
}

int DBImpl::Put(std::string key, std::string _)
{
    int ok;

    padKey(key);

    memTable_->Put(key, _);
    if (memTable_->Full()) {
        ok = flush();
        if (ok == -1) {
            return -1;
        }
    }
    return 0;
}

std::string DBImpl::ToString()
{
    int ok;
    std::ostringstream oss;

    oss << manifest_->ToString();

    for (uint32_t i = 0; i < ssts_.size(); i++) {
        oss << "Level " << i << ": ";
        std::set<std::shared_ptr<SST>, SST::SSTComparator>::iterator it;
        for (it = ssts_.at(i).begin(); it != ssts_.at(i).end(); it++) {
            oss << (*it).get()->GetName() << " ";
        }
        oss << "\n";
    }

    return oss.str();
}

int DBImpl::populate(int n)
{
    printf("Populating DBImpl with %d entries\n", n);
    std::shared_ptr<SST> sst;
    int ok;

    for (int i = 0; i < n; i++) {
        sst = manager_->NewEmptySST();
        if (sst == nullptr) {
            return -1;
        }

        ok = manager_->PopulateSST(*sst);
        if (ok == -1) {
            return -1;
        }

        ssts_.at(0).insert(sst);
        manifest_->AddToLevel(i, sst.get()->GetID());
    }

    ok = manifest_->Persist();
    return 0;
}

bool DBImpl::verifyConfig()
{
    assert(config_->sst_file_size % (uint64_t)config_->kv_size() == 0);
    assert(BLOCK_SIZE % config_->index_block_entry_size() == 0);
    return true;
}

int DBImpl::flush()
{
    printf("Triggered flush\n");

    std::shared_ptr<SST> sst;
    int ok;

    ok = manager_->FlushToSST(memTable_, sst);
    if (ok == -1) {
        return -1;
    }

    memTable_->Flush();
    manifest_->AddToLevel(0, sst.get()->GetID());
    ssts_.at(0).insert(sst);

    if (ssts_.at(0).size() > 1) {
        ok = compact();
    }
    else {
        ok = manifest_->Persist();
    }

    printf("%s\n", ToString().c_str());

    return ok;
}

int DBImpl::compact()
{
    printf("Triggered compaction\n");
    int ok;

    ok = compacter_->Compact();
    if (ok == -1) {
        fprintf(stderr, "error during compaction\n");
        return -1;
    }

    ok = manifest_->Persist();
    if (ok == -1) {
        return -1;
    }

    return 0;
}

void DBImpl::padKey(std::string& key)
{
    key.append(std::string(config_->key_size - key.size(), '_'));
}
