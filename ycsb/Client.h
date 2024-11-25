#pragma once

#include "../DB.h"
#include "../include/config.h"
#include "../utils/utils.h"
#include "WorkloadProducer.h"

/**
 * This code is inspired by and adapted from
 * https://github.com/SpanDB/SpanDB/blob/master/ycsb/src/rocksdb_client.h
 */

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

    TimeRecord request_time_;
    TimeRecord read_time_;
    TimeRecord write_time_;
    TimeRecord scan_time_;
};
}  // namespace ycsbc