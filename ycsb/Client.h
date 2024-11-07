#pragma once

#include <sys/time.h>

#include "../DB.h"
#include "../include/config.h"
#include "WorkloadProducer.h"

/**
 * This code is inspired by and adapted from
 * https://github.com/SpanDB/SpanDB/blob/master/ycsb/src/rocksdb_client.h
 */

#define TIME_NOW (std::chrono::high_resolution_clock::now())
#define TIME_DURATION(start, end) \
    (std::chrono::duration<double>((end) - (start)).count() * 1000 * 1000)
typedef std::chrono::high_resolution_clock::time_point TimePoint;

namespace ycsbc {
class Client {
   public:
    Client(Config* config);
    ~Client();
    int Load();
    int Work();

   private:
    static std::string GetDayTime()
    {
        const int kBufferSize = 100;
        char buffer[kBufferSize];
        struct timeval now_tv;
        gettimeofday(&now_tv, nullptr);
        const time_t seconds = now_tv.tv_sec;
        struct tm t;
        localtime_r(&seconds, &t);
        snprintf(buffer, kBufferSize, "%04d/%02d/%02d-%02d:%02d:%02d.%06d",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
                 static_cast<int>(now_tv.tv_usec));
        return std::string(buffer);
    }

    struct TimeRecord {
       private:
        std::vector<double> data_;
        bool sorted;

       public:
        TimeRecord() : sorted(false) {}
        ~TimeRecord() {}
        void Insert(double time)
        {
            data_.emplace_back(time);
            sorted = false;
        }
        std::vector<double> Data()
        {
            return data_;
        }
        double Sum()
        {
            return std::accumulate(data_.begin(), data_.end(), 0.0);
        }
        uint64_t Size()
        {
            return data_.size();
        }
        double Tail(double f)
        {
            if (!sorted) {
                std::sort(data_.begin(), data_.end());
                sorted = true;
            }
            return data_[(uint64_t)(data_.size() * f)];
        }
    };

    Config* config_;
    DB* db_;
    WorkloadProducer* producer_;
    CoreWorkload* wl_;
    WorkloadProxy* proxy_;
    utils::Properties props_;

    TimeRecord request_time_;
    TimeRecord read_time_;
    TimeRecord write_time_;
};
}  // namespace ycsbc