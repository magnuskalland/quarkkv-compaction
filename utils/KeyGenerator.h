#pragma once

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

class KeyGenerator {
   public:
    KeyGenerator(size_t length) : length_(length), gen_(std::random_device{}()) {}
    virtual std::string Generate() = 0;

   protected:
    size_t length_;
    std::mt19937 gen_;
    std::string chars_ = "abcdefghijklmnopqrstuvwxyz";
};

class UniformKeyGenerator : public KeyGenerator {
   public:
    UniformKeyGenerator(size_t length);
    std::string Generate() override;
};

class ZipfianKeyGenerator : public KeyGenerator {
   public:
    ZipfianKeyGenerator(size_t length);
    std::string Generate() override;
};
