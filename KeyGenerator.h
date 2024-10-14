#pragma once

#include <cstdint>
#include <string>

class KeyGenerator {
   public:
    KeyGenerator(uint64_t keyLen, uint64_t start = 0);

    std::string Generate(int *keyValue);
    std::string GenerateRand(int *keyValue);

   private:
    std::string pad(std::string string);
    uint64_t counter_;
    uint64_t key_length_;
};