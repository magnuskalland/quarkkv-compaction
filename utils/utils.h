#pragma once

#include <stdint.h>
#include <sys/time.h>

#define TIME_NOW (std::chrono::high_resolution_clock::now())
#define TIME_DURATION(start, end) \
    (std::chrono::duration<double>((end) - (start)).count() * 1000 * 1000)
typedef std::chrono::high_resolution_clock::time_point TimePoint;

inline int round_up_to_nearest_multiple(uint64_t from, uint64_t to)
{
    return ((from + to - 1) / to) * to;
}

inline std::string GetDayTime()
{
    const int kBufferSize = 100;
    char buffer[kBufferSize];
    struct timeval now_tv;
    gettimeofday(&now_tv, nullptr);
    const time_t seconds = now_tv.tv_sec;
    struct tm t;
    localtime_r(&seconds, &t);
    snprintf(buffer, kBufferSize, "%04d/%02d/%02d-%02d:%02d:%02d.%06d", t.tm_year + 1900,
             t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
             static_cast<int>(now_tv.tv_usec));
    return std::string(buffer);
}

struct TimeRecord {
   private:
    std::vector<double> data_;
    bool sorted_;

   public:
    TimeRecord() : sorted_(false) {}
    ~TimeRecord() {}
    void Clear()
    {
        data_.clear();
        sorted_ = false;
    }
    void Insert(double time)
    {
        data_.emplace_back(time);
        sorted_ = false;
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
    void Join(struct TimeRecord other)
    {
        data_.insert(data_.begin(), other.Data().begin(), other.Data().end());
        sorted_ = false;
    }
    double Tail(double f)
    {
        if (!sorted_) {
            std::sort(data_.begin(), data_.end());
            sorted_ = true;
        }
        return data_[(uint64_t)(data_.size() * f)];
    }
};