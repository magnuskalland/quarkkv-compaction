#pragma once

#include <atomic>

#include "../include/SSTManager.h"

class SSTManagerFS : public SSTManager {
   public:
    SSTManagerFS(Config* config);
    ~SSTManagerFS() override;
    std::shared_ptr<SST> NewEmptySST() override;
    int PopulateSST(SST& sst) override;
    std::shared_ptr<SST> ReadSST(uint32_t id) override;

    int Get(SST* sst, std::string key, KVPair** dest) override;

   private:
    std::atomic<int> ctr_;
};