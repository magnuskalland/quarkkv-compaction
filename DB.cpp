
#include "include/DB.h"

#include <cassert>
#include <iterator>

#include "fs/SSTManagerFS.h"
#include "include/SST.h"

DB::DB(Config* config) : config_(config)
{
    manifest_ = new ManifestFS(config_);
    manager_ = new SSTManagerFS(config_);
    memTable_ = new MemTable(config_->sst_file_size / config_->kv_size());
    for (uint32_t level = 0; level < config_->n_levels; level++) {
        ssts_.emplace_back(std::vector<std::shared_ptr<SST>>());
    }
};

DB::~DB()
{
    delete manifest_;
    delete manager_;
}

void DB::VerifyConfig()
{
    assert(config_->sst_file_size % (uint64_t)config_->kv_size() == 0);
    assert(BLOCK_SIZE % config_->index_block_entry_size() == 0);
}

int DB::Open()
{
    printf("Opening DB\n");
    int ok;
    ok = manifest_->Open();
    if (ok != 0) {
        return -1;
    }

    std::vector<int> ssts = manifest_->FlattenSSTs();
    printf("Parsing %ld SST files from manifest\n", ssts.size());
    std::vector<int>::iterator it;
    for (it = ssts.begin(); it != ssts.end(); it++) {
        std::shared_ptr<SST> sst = manager_->ReadSST(*it);
        if (sst == nullptr) {
            return -1;
        }
        ssts_.at(*it).emplace_back(sst);
    }

    return 0;
}

int DB::Close()
{
    std::vector<std::vector<std::shared_ptr<SST>>>::iterator outer;
    std::vector<std::shared_ptr<SST>>::iterator it;
    for (outer = ssts_.begin(); outer != ssts_.end(); ++outer) {
        for (it = outer->begin(); it != outer->end(); ++it) {
            (*it).get()->Close();
        }
    }
    return 0;
}

int DB::Get(std::string key, std::string& dest)
{
    int ok;
    std::string* val;

    val = memTable_->Get(key);
    if (val) {
        printf("Found key in memtable\n");
        return 0;
    }

    KVPair *kv, *latest = nullptr;
    SST* sst;
    std::vector<std::shared_ptr<SST>> level0 = ssts_.at(0);

    std::vector<std::shared_ptr<SST>>::iterator it;
    for (it = level0.begin(); it != level0.end(); ++it) {
        sst = (*it).get();
        ok = manager_->Get(sst, key, &kv);
        if (ok == -1) {
            return -1;
        }

        if (kv) {
            if (!latest || kv->Compare(latest)) {
                latest = kv;
            }
        }
    }

    dest = latest->ToString();
    return 0;
}

int DB::Put(std::string key, std::string _)
{
    int ok;
    memTable_->Put(key, _);
    if (memTable_->Full()) {
        ok = Compact();
        if (ok == -1) {
            return -1;
        }
    }
    return 0;
}

int DB::Populate(int n)
{
    printf("Populating DB with %d entries\n", n);
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

        ssts_.at(0).emplace_back(sst);
        manifest_->AddToLevel(i, sst.get()->GetID());
    }

    ok = manifest_->Persist();
    return 0;
}

int DB::Compact()
{
    printf("Triggered compaction\n");
    while (1)
        ;

    return 0;
}