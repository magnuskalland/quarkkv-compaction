#include "../include/SSTManager.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <string>

#include "SSTFS.h"

SSTManager::SSTManager(Config* config)
    : config_(config), keygen_(new UniformKeyGenerator(config->key_size)), ctr_(0)
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
        std::shared_ptr<SST> sst = SSTFS::CreateNewEmpty(config_, id);
        if (sst != nullptr) {
            return sst;
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
        ok = sst.AddKey(key);
        if (ok == -1) {
            return -1;
        }
    }

    ok = sst.Persist();
    if (ok == -1) {
        return -1;
    }

    return 0;
}

std::shared_ptr<SST> SSTManager::ReadSST(uint32_t id)
{
    return SSTFS::OpenWithID(config_, id);
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
    kvs = table->GetTable();

    assert(kvs->size() == config_->sst_file_size / config_->kv_size());

    for (auto it = kvs->begin(); it != kvs->end(); ++it) {
        ok = sst->AddKV(&(*it).second);
        if (ok == -1) {
            return -1;
        }
    }

    ok = sst->Persist();
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