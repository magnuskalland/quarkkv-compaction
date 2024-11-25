#pragma once

#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

class KVPair {
   public:
    struct KVPairComparator {
        bool operator()(const KVPair& lhs, const KVPair& rhs) const
        {
            return lhs > rhs;
        }
    };

    KVPair(std::string key, uint64_t ts, uint32_t value_size)
        : key_(key), ts_(ts), value_size_(value_size)
    {
    }

    KVPair(std::string key, uint64_t ts, uint32_t value_size, int handler, off_t offset)
        : key_(key), ts_(ts), value_size_(value_size), handler_(handler), offset_(offset)
    {
    }

    std::string GetKey() const
    {
        return key_;
    }

    uint64_t GetTimestamp() const
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
        std::string tsString = tsToString();
        std::string value(value_size_ - tsString.length(), 'V');
        return key_ + value + tsString;
    }

    std::string ToString() const
    {
        std::string value(" (value) ");
        return key_ + value + tsToString();
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
    uint64_t ts_;
    uint32_t value_size_;

    // info for QuarkStore
    int handler_ = -1;
    off_t offset_ = -1;

    std::string tsToString() const
    {
        std::ostringstream oss;
        oss << "0x" << std::setw(16) << std::setfill('0') << std::hex << std::nouppercase
            << static_cast<uint64_t>(ts_);
        return oss.str();
    }
};
