#pragma once

#include <atomic>

#include "../include/SSTManager.h"

class SSTManagerFS : public SSTManager {
   public:
    SSTManagerFS(Config* config);
    ~SSTManagerFS() override;
    std::shared_ptr<SST> NewEmptySST() override;
    int PopulateSST(SST& sst) override;
    int ReadSST(uint32_t id, uint32_t level) override;

   private:
    std::atomic<int> ctr_;
};