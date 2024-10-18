#pragma once

#include <stdint.h>

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "KVPair.h"
#include "config.h"

class SST {
   public:
    virtual ~SST() = default;

    /**
     * Factory method. Creates a new, empty SST file
     * @param id ID of SST file. Must not already exist.
     */
    std::shared_ptr<SST> CreateNewEmpty(std::string dir, uint32_t id);

    /**
     * Factory method. Opens a persisted SST file with ID.
     * @param id ID of existing SST file.
     */
    std::shared_ptr<SST> OpenWithID(Config* config, uint32_t id);

    /**
     * Persist SST file to disk.
     */
    virtual int Persist() = 0;

    /**
     * Parse an SST file from disk.
     */
    virtual int Parse() = 0;

    virtual int Close() = 0;

    virtual int Get(std::string key, KVPair** dest) = 0;

    int GetHandler()
    {
        return handler_;
    }

    uint32_t GetEntries()
    {
        return entries_;
    }

    std::string GetSmallestKey()
    {
        return smallestKey_;
    }

    std::string GetLargestKey()
    {
        return largestKey_;
    }

    int GetID()
    {
        return id_;
    }

   protected:
    SST(Config* config, uint32_t handler, int id) : config_(config), handler_(handler), id_(id) {}

    virtual int writeIndexBlock() = 0;
    virtual int writeNumberOfEntries() = 0;
    virtual int readIndexBlock() = 0;
    virtual int readNumberOfEntries() = 0;
    virtual int readKVAtOffset(uint64_t off, KVPair** kv_pair) = 0;

    Config* config_;
    int handler_;
    int id_;
    uint32_t entries_;
    std::string smallestKey_;
    std::string largestKey_;
    std::map<std::string, uint64_t> indexTable_;

    static std::string createNameFromID(uint32_t id)
    {
        std::ostringstream oss;
        oss << std::setw(6) << std::setfill('0') << id << ".sst";
        return oss.str();
    }

   private:
};