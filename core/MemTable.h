#pragma once

#include <map>
#include <string>

#include "KVPair.h"
#include "config.h"

class MemTable {
   public:
    MemTable(Config* config);
    KVPair* Get(std::string key);
    void Put(std::string key, std::string val);
    bool Full();
    std::map<std::string, KVPair>* GetTable();
    void Flush();

   private:
    Config* config_;
    std::map<std::string, KVPair> table_;
    size_t limit_;
};