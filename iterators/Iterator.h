#pragma once

#include <vector>

#include "../include/SST.h"
#include "../include/config.h"

class Iterator {
   public:
    Iterator(Config* config) : config_(config) {}
    virtual void Next() = 0;

    bool operator>(const Iterator& other) const
    {
        assert(ptr_ && other.ptr_);
        return ptr_ > other.ptr_;
    }

    KVPair* Get() const
    {
        return ptr_;
    }

    KVPair* End() const
    {
        return nullptr;
    }

   protected:
    Config* config_;
    KVPair* ptr_ = nullptr;
};