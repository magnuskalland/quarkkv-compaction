#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../utils/KeyGenerator.h"
#include "config.h"

class SSTFileManager {
   public:
    SSTFileManager(Config* config) : config_(config), keygen_(new UniformKeyGenerator(config->key_size)) {}
    virtual ~SSTFileManager() {}
    virtual int CreateNewSST(uint32_t id) = 0;
    virtual int PopulateSST(uint32_t id) = 0;

   protected:
    std::string get_name(uint32_t id)
    {
        std::ostringstream oss;
        oss << std::setw(6) << std::setfill('0') << id << ".sst";
        return oss.str();
    }

    std::string getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_c);

        std::ostringstream oss;
        oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::time_t parseTimestamp(const std::string& timestamp)
    {
        std::tm tm = {};
        std::istringstream ss(timestamp);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

        if (ss.fail()) {
            throw std::runtime_error("Failed to parse timestamp");
        }

        // Convert to time_t
        return std::mktime(&tm);
    }

    Config* config_;
    KeyGenerator* keygen_;
};