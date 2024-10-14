#include "KeyGenerator.h"

#include <cstdlib>
#include <iomanip>
#include <sstream>

KeyGenerator::KeyGenerator(uint64_t keyLen, uint64_t start) : key_length_(keyLen), counter_(start) {}

std::string KeyGenerator::pad(std::string string) { return string + std::string(key_length_ - string.length(), '!'); }

std::string KeyGenerator::Generate(int *keyValue) {
    *keyValue = counter_++;
    std::stringstream ss;
    ss << std::setw(10) << std::setfill('0') << *keyValue;
    return pad("key" + ss.str());
}

std::string KeyGenerator::GenerateRand(int *keyValue) {
    *keyValue = rand();
    std::stringstream ss;
    ss << std::setw(10) << std::setfill('0') << *keyValue;
    return pad("key" + ss.str());
}