#pragma once

#include "../DB.h"
#include "../include/config.h"
#include "WorkloadProducer.h"

namespace ycsbc {
class Client {
   public:
    Client(Config* config);
    ~Client();
    int Load();
    int Work();

   private:
    Config* config_;
    DB* db_;
    WorkloadProducer* producer_;
    CoreWorkload* wl_;
    WorkloadProxy* proxy_;
    utils::Properties props_;
};
}  // namespace ycsbc