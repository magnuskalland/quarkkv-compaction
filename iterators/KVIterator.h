#pragma once

#include "../include/Iterator.h"
#include "../include/SST.h"
#include "string"

class KVIterator : public Iterator {
   public:
    KVIterator(Config* config, std::shared_ptr<SST> sst);
    void Next() override;
    int SeekToFirst() override;
    int Seek(std::string key);

   protected:
    std::shared_ptr<SST> sst_;
    KVPair* end_;
    uint32_t index_ = 0;
};
