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
    struct SSTComparator {
        bool operator()(const std::shared_ptr<SST> s1,
                        const std::shared_ptr<SST> s2) const
        {
            if ((s1.get()->GetSmallestKey().compare(s2.get()->GetSmallestKey()) == 0)) {
                return s1.get()->GetID() < s2.get()->GetID();
            }
            return s1.get()->GetSmallestKey().compare(s2.get()->GetSmallestKey()) < 0;
        }
    };

    virtual ~SST();
    virtual std::string GetName() = 0;
    virtual bool operator>(const SST& other) const;

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
    int Persist();

    /**
     * Remove SST from underlying storage.
     * @return -1 if error, 0 otherwise.
     */
    virtual int Remove() = 0;

    /**
     * Parse an SST file from disk.
     */
    int Parse();

    /**
     * Look up the KV-pair associated with key.
     * @param key Key to look up.
     * @param dest Destination pointer to place put the KV-pair.
     * @return -1 if error, 0 otherwise.
     */
    int Lookup(std::string key, KVPair** dest);

    /**
     * Add key to SST. Will create a new timestamp to
     * associate with key insertion time.
     * @param key Key to insert.
     * @return -1 if error, 0 otherwise.
     */
    int AddKey(std::string key);

    /**
     * Add KV-pair to SST. Used during compaction and flushing to
     * retain the old timestamp.
     * @param kv KV-pair to insert.
     * @return -1 if error, 0 otherwise.
     */
    int AddKV(KVPair* kv);

    int GetKVAtIndex(uint32_t index, KVPair** dest);

    int GetHandler();
    uint32_t GetEntries();
    uint32_t GetLevel();
    void SetLevel(uint32_t level);
    std::string GetSmallestKey();
    std::string GetLargestKey();
    int GetID();
    bool IsMarkedForCompaction();
    void MarkForCompaction();
    void UnmarkForCompaction();
    bool IsFull();
    bool IsPersisted();
    std::chrono::system_clock::time_point GetPersistTime();

   protected:
    /**
     * Constructor.
     */
    SST(Config* config, uint32_t handler, int id);

    // I/O wrappers

    /**
     * Append to SST file. Must specify offset.
     * @param buf Data to append.
     * @param size Length of data to append
     * @return -1 if error, 0 otherwise.
     */
    virtual int append(char* buf, size_t size) = 0;

    /**
     * Read from SST file.
     * @param buf Destination buffer of read.
     * @param offset Offset of file to read from.
     * @param size Length of read.
     * @return -1 if error, 0 otherwise.
     */
    virtual int read(char* buf, size_t size, off_t offset) = 0;

    // virtual bool verifyPersisted() = 0;

    // Private objects

    Config* config_;
    int handler_;
    int id_;
    std::string smallestKey_;
    std::string largestKey_;
    std::map<std::string, uint64_t> indexTable_;
    std::chrono::system_clock::time_point ts_;

    uint32_t entries_ = 0;
    uint32_t level_ = 0;
    bool persisted_ = false;
    bool markedForCompaction_ = false;

    uint32_t indexBlockSize_ = -1;
    off_t dataBlockOffset_ = -1;
    off_t indexBlockOffset_ = -1;
    off_t numberOfEntriesOffset_ = -1;

    int appendKV(KVPair* kv);
    int writeIndexBlock();
    int writeNumberOfEntries();
    int readKV(off_t offset, KVPair** dest);
    int readIndexBlock();
    int readNumberOfEntries();
};