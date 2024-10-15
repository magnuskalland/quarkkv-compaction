#include "SSTManagerFS.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <string>

#include "../include/Manifest.h"
#include "SSTFS.h"
#include "io.h"

SSTManagerFS::SSTManagerFS(Config* config) : SSTManager(config), ctr_(0) {}
SSTManagerFS::~SSTManagerFS() {}

std::shared_ptr<SST> SSTManagerFS::NewEmptySST()
{
    int id;
    while (true) {
        id = ctr_.fetch_add(1);
        std::shared_ptr<SST> sst = SSTFS::CreateNewEmpty(config_, id);
        if (sst != nullptr) {
            return sst;
        }
    }
}

int SSTManagerFS::PopulateSST(SST& sst)
{
    int ok;
    SSTFS* sstfs = dynamic_cast<SSTFS*>(&sst);

    // generate keys and add them to a sorted container
    std::set<std::string> keys;
    size_t total_number_of_keys_in_sst = config_->sst_file_size / config_->kv_size();
    while (keys.size() < total_number_of_keys_in_sst) {
        std::string key = keygen_->Generate();
        keys.emplace(key);
    }

    // add keys to SST file
    for (const std::string& key : keys) {
        ok = sstfs->Append(key);
        if (ok == -1) {
            return -1;
        }
    }

    ok = sstfs->Persist();
    if (ok == -1) {
        return -1;
    }

    return 0;
}

int SSTManagerFS::ReadSST(uint32_t id, uint32_t level)
{
    return 0;
}