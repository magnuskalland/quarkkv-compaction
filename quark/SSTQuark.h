#pragma once

#include "../include/SST.h"

class SSTQuark : public SST {
   public:
    ~SSTQuark();
    static std::shared_ptr<SST> CreateNewEmpty(Config* config, uint32_t aid);
    static std::shared_ptr<SST> OpenWithID(Config* config, uint32_t aid);
    int Remove() override;
    int MoveAndAddKV(KVPair* kv);

   protected:
    SSTQuark(Config* config, uint32_t handler, int aid);
    int append(char* buf, size_t size) override;
    int read(char* buf, size_t size, off_t offset) override;

   private:
    off_t atomSize_ = 0;
};