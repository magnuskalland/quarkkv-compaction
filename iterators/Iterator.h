#pragma once

#include <vector>

#include "../include/SST.h"
#include "../include/config.h"

class Iterator {
   public:
    Iterator(Config* config) : config_(config) {}
    virtual void Next() = 0;
    virtual int SeekToFirst() = 0;

    struct Comparator {
        bool operator()(const std::shared_ptr<Iterator>& lhs,
                        const std::shared_ptr<Iterator>& rhs) const
        {
            return *lhs.get()->Get() > *rhs.get()->Get();
        }
    };

    KVPair* Get() const
    {
        return ptr_;
    }

    KVPair* End() const
    {
        return nullptr;
    }

    uint32_t GetIterations() const
    {
        return iterations_;
    }

   protected:
    Config* config_;
    KVPair* ptr_ = nullptr;
    uint32_t iterations_ = 0;
};