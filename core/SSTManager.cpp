#include "../include/SSTManager.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <string>

#include "../fs/SSTFS.h"
#include "../quark/SSTQuark.h"

SSTManager::SSTManager(Config* config)
    : config_(config),
      keygen_(new UniformKeyGenerator(config->key_size)),
      ctr_(config->engine == FS ? 0 : config->quarkstore_sst_aid_start),
      clock_(0)
{
}
SSTManager::~SSTManager() {}

std::shared_ptr<SST> SSTManager::NewEmptySST()
{
    assert(config_);
    assert(ctr_ >= 0);
    int id;
    while (true) {
        id = ctr_.fetch_add(1);
        if (config_->engine == FS) {
            std::shared_ptr<SST> sst = SSTFS::CreateNewEmpty(config_, id);
            if (sst != nullptr) {
                return sst;
            }
        }
        else if (config_->engine == QUARKSTORE || config_->engine == QUARKSTORE_APPEND) {
            uint64_t t = id;
            return SSTQuark::CreateNewEmpty(config_, &t);
        }
    }
}

int SSTManager::PopulateSST(SST& sst)
{
    int ok;

    // generate keys and add them to a sorted container
    std::set<std::string> keys;
    size_t total_number_of_keys_in_sst = config_->sst_file_size / config_->kv_size();
    while (keys.size() < total_number_of_keys_in_sst) {
        std::string key = keygen_->Generate();
        keys.emplace(key);
    }

    // add keys to SST file
    for (const std::string& key : keys) {
        ok = sst.AddKey(key, ClockGetAndIncrement());
        if (ok == -1) {
            return -1;
        }
    }

    ok = sst.Persist(ClockGetAndIncrement());
    if (ok == -1) {
        return -1;
    }

    return 0;
}

std::shared_ptr<SST> SSTManager::ReadSST(uint32_t id)
{
    if (config_->engine == FS) {
        return SSTFS::OpenWithID(config_, id);
    }
    else if (config_->engine == QUARKSTORE || config_->engine == QUARKSTORE_APPEND) {
        // we don't care about atomicity
        if ((int)id >= ctr_.load()) {
            ctr_.store(id + 1);
        }
        uint64_t t = id;
        return SSTQuark::OpenWithID(config_, &t);
    }
    fprintf(stderr, "engine undefined");
    return nullptr;
}

int SSTManager::Get(SST* sst, std::string key, KVPair** dest)
{
    return sst->Lookup(key, dest);
}

int SSTManager::FlushToSST(MemTable* table, std::shared_ptr<SST>& sst)
{
    int ok;
    std::map<std::string, KVPair>* kvs;

    sst = NewEmptySST();
    assert(sst);
    if (!sst) {
        return -1;
    }
    kvs = table->GetTable();
    assert(kvs->size() == config_->sst_file_size / config_->kv_size());

    for (auto it = kvs->begin(); it != kvs->end(); ++it) {
        ok = sst->AddKV(&(*it).second);
        if (ok == -1) {
            return -1;
        }
    }

    ok = sst->Persist(ClockGetAndIncrement());
    if (ok == -1) {
        return -1;
    }

    return 0;
}

int SSTManager::RemoveSST(std::shared_ptr<SST>& sst)
{
    if (!sst.get()->IsPersisted()) {
        return 0;
    }

    int ok = sst.get()->Remove();
    if (ok == -1) {
        return -1;
    }

    return 0;
}

uint64_t SSTManager::ClockGetAndIncrement()
{
    return clock_++;
}