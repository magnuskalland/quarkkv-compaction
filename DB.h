#pragma once

#include <string>

#include "include/DBImpl.h"

class DB {
   public:
    DB(Config* config);
    int Open();
    int Close();
    int Get(std::string key, std::string& dest);
    int Put(std::string key, std::string _);
    int Scan(std::string start, int len,
             std::map<std::string, std::string>::iterator& dest);
    std::string ToString();

    DBImpl::stats GetStats();
    void ClearStats();

   private:
    DBImpl db_;
};