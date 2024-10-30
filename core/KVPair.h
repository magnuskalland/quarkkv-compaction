#pragma once

#include <cassert>
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
        ts_ = now;
    }

    KVPair(std::string key, std::string ts, uint32_t value_size)
        : key_(key), value_size_(value_size)
    {
        parseTS(ts);
    }

    KVPair(std::string key, std::string ts, uint32_t value_size, int handler,
           off_t offset)
        : key_(key), value_size_(value_size), handler_(handler), offset_(offset)
    {
        parseTS(ts);
    }

    std::string GetKey() const
    {
        return key_;
    }

    std::chrono::system_clock::time_point GetTimestamp() const
    {
        return ts_;
    }

    bool operator>(const KVPair& other) const
    {
        int keyComp = key_.compare(other.GetKey());
        if (keyComp != 0) {
            return keyComp > 0 ? true : false;
        }
        auto otherTS = other.GetTimestamp();
        return ts_ > otherTS;
    }

    std::string ToBinary() const
    {
        std::string ts = tsToString();
        std::string value(value_size_ - ts.length(), 'V');
        return key_ + value + ts;
    }

    std::string ToString() const
    {
        std::string ts = tsToString();
        std::string value(" (value) ");
        return key_ + value + ts;
    }

    int GetHandler()
    {
        return handler_;
    }

    off_t GetOffset()
    {
        return offset_;
    }

   private:
    std::string key_;
    std::chrono::system_clock::time_point ts_;
    uint32_t value_size_;

    // info for QuarkStore
    int handler_ = -1;
    off_t offset_ = -1;

    std::string tsToString() const
    {
        auto in_time_t = std::chrono::system_clock::to_time_t(ts_);
        auto microsec = std::chrono::duration_cast<std::chrono::microseconds>(
                            ts_.time_since_epoch()) %
                        1000000;

        std::ostringstream oss;
        std::tm* now_tm = std::localtime(&in_time_t);
        oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
        oss << "." << std::setw(6) << std::setfill('0') << microsec.count();
        return oss.str();
    }

    void parseTS(std::string ts)
    {
        std::tm tm = {};
        std::istringstream ss(ts);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto base_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        size_t dotPos = ts.find('.');
        if (dotPos != std::string::npos) {
            int microseconds = std::stoi(ts.substr(dotPos + 1, 6));
            ts_ = base_time + std::chrono::microseconds(microseconds);
        }
        else {
            ts_ = base_time;
        }
    }
};
