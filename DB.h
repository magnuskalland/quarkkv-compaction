#pragma once

#include <set>

#include "include/Manifest.h"
#include "include/MemTable.h"
#include "include/SSTManager.h"
#include "include/config.h"

class DB {
   public:
    DB(Config* config);
    ~DB();
    int Open();
    int Close();
    int Get(std::string key, std::string& dest);
    int Put(std::string key, std::string _);

    void VerifyConfig();
    int Populate(int n);
    std::string ToString();

   private:
    Manifest* manifest_;
    SSTManager* manager_;
    Config* config_;
    MemTable* memTable_;
    std::vector<std::vector<std::shared_ptr<SST>>> ssts_;

    int compact();
    int flush();
    void padKey(std::string& key);
};