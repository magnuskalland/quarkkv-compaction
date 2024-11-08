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
    std::string ToString();

    DBImpl::stats GetStats();
    void ClearStats();

   private:
    DBImpl db_;
};