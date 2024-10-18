#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

class KVPair {
   public:
    KVPair(std::string key, uint32_t value_size) : key_(key), value_size_(value_size)
    {
        auto now = std::chrono::system_clock::now();
        ts_ = std::chrono::system_clock::to_time_t(now);
    }

    KVPair(std::string key, std::string ts, uint32_t value_size) : key_(key), value_size_(value_size)
    {
        std::tm tm = {};
        std::istringstream ss(ts);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        ts_ = std::mktime(&tm);
    }

    std::string GetKey()
    {
        return key_;
    }

    std::time_t GetTimestamp()
    {
        return ts_;
    }

    int Compare(KVPair* other)
    {
        int keyComp = key_.compare(other->GetKey());
        std::time_t otherTS = other->GetTimestamp();
        int tsComp = ts_ < otherTS ? -1 : ts_ == otherTS ? 0 : 1;
        return keyComp < 0 ? 0 : keyComp > 0 ? 1 : tsComp;
    }

    std::string ToString()
    {
        std::ostringstream oss;
        std::tm* now_tm = std::localtime(&ts_);
        oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
        std::string ts = oss.str();
        std::string value(value_size_ - ts.length(), 'V');
        return key_ + value + ts;
    }

   private:
    std::string key_;
    std::time_t ts_;
    uint32_t value_size_;
};