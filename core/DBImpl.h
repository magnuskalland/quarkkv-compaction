#pragma once

#include <set>

#include "../include/Compacter.h"
#include "../include/Manifest.h"
#include "../include/MemTable.h"
#include "../include/SSTManager.h"
#include "../include/config.h"

class DBImpl {
   public:
    struct stats {
        TimeRecord compactionTimes;
    } stats_;

    // Public interface
    DBImpl(Config* config);
    int Open();
    int Close();
    int Get(std::string key, std::string& dest);
    int Put(std::string key, std::string _);
    int Scan(std::string start, int len,
             std::map<std::string, std::string>::iterator& dest);

    std::string ToString();
    struct stats GetStats();
    void ClearStats();

   private:
    Manifest* manifest_;
    SSTManager* manager_;
    Config* config_;
    MemTable* memTable_;
    Compacter* compacter_;
    uint32_t flushes_ = 0;
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>> ssts_;

    int populate(int n);
    int compact();
    int flush();
    void padKey(std::string& key);
};