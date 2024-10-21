
#include "../DB.h"

#include <cassert>
#include <chrono>
#include <iterator>
#include <thread>

#include "../fs/ManifestFS.h"

DB::DB(Config* config) : config_(config)
{
    manifest_ = new ManifestFS(config_);
    manager_ = new SSTManager(config_);
    memTable_ = new MemTable(config_);
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

    std::vector<std::vector<int>> ssts = manifest_->GetSSTs();
    for (uint32_t i = 0; i < ssts.size(); i++) {
        std::vector<int>::iterator it;
        std::vector<int> level = ssts.at(i);
        for (it = level.begin(); it != level.end(); it++) {
            std::shared_ptr<SST> sst = manager_->ReadSST(*it);
            if (sst == nullptr) {
                return -1;
            }
            ssts_.at(i).emplace_back(sst);
        }
    }

    return 0;
}

int DB::Close()
{
    std::vector<std::vector<std::shared_ptr<SST>>>::iterator outer;
    std::vector<std::shared_ptr<SST>>::iterator it;
    for (outer = ssts_.begin(); outer != ssts_.end(); ++outer) {
        for (it = outer->begin(); it != outer->end(); ++it) {
            delete (*it).get();
        }
    }
    return 0;
}

int DB::Get(std::string key, std::string& dest)
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
    std::vector<std::shared_ptr<SST>> level0 = ssts_.at(0);

    // search in level 0
    std::vector<std::shared_ptr<SST>>::iterator it;
    for (it = level0.begin(); it != level0.end(); ++it) {
        sst = (*it).get();
        ok = manager_->Get(sst, key, &kv);
        if (ok == -1) {
            return -1;
        }

        if (kv && (!latest || kv->Compare(latest))) {
            latest = kv;
        }
    }

    if (latest) {
        printf("Found KV at level 0\n");
        dest = latest->ToString();
        return 0;
    }

    // search in rest of levels
    std::vector<std::vector<std::shared_ptr<SST>>>::iterator outer = ssts_.begin();
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

int DB::Put(std::string key, std::string _)
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

std::string DB::ToString()
{
    int ok;
    std::ostringstream oss;

    oss << manifest_->ToString();

    for (uint32_t i = 0; i < ssts_.size(); i++) {
        oss << "Level " << i << ": ";
        for (uint32_t j = 0; j < ssts_.at(i).size(); j++) {
            oss << ssts_.at(i).at(j).get()->GetName() << " ";
        }
        oss << "\n";
    }

    return oss.str();
}

int DB::flush()
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
    ssts_.at(0).emplace_back(sst);

    if (ssts_.at(0).size() > 1) {
        ok = compact();
    }
    else {
        ok = manifest_->Persist();
    }

    printf("%s\n", ToString().c_str());
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return ok;
}

int DB::compact()
{
    printf("Triggered compaction\n");
    int ok;
    // while (1)
    //     ;

    // TODO:

    ok = manifest_->Persist();
    if (ok == -1) {
        return -1;
    }
    return 0;
}

void DB::padKey(std::string& key)
{
    key.append(std::string(config_->key_size - key.size(), '_'));
}
