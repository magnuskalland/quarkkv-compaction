
#include "include/DB.h"

#include <cassert>

#include "fs/SSTManagerFS.h"
#include "include/SST.h"

DB::DB(Config* config) : config_(config)
{
    manifest_ = new ManifestFS(config_);
    manager_ = new SSTManagerFS(config_);
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
    return 0;
}

int Close()
{
    return 0;
}

int DB::Get(std::string key)
{
    return 0;
}

int DB::Put(std::string key, std::string _)
{
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

        printf("SST pointer to SST %d is 0x%p\n", sst.get()->GetID(), sst.get());

        ok = manager_->PopulateSST(*sst);
        if (ok == -1) {
            return -1;
        }

        ssts_.emplace_back(sst);
        manifest_->AddToLevel(i, sst.get()->GetID());
    }

    ok = manifest_->Persist();
    return 0;
}