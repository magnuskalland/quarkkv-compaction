#include "SSTFileManagerFS.h"

#include <cassert>
#include <cstring>
#include <string>

#include "../include/Manifest.h"
#include "io.h"

SSTFileManagerFS::SSTFileManagerFS(Config* config) : SSTFileManager(config) {}
SSTFileManagerFS::~SSTFileManagerFS() {}

int SSTFileManagerFS::CreateNewSST(uint32_t id)
{
    std::string name = get_name(id);
    std::string full_path = config_->ddir + "/" + name;
    return Create(full_path);
}

int SSTFileManagerFS::PopulateSST(uint32_t id)
{
    int fd, ok;
    std::string name = get_name(id);
    std::string full_path = config_->ddir + "/" + name;

    fd = Open(full_path);
    if (fd < 0) {
        return -1;
    }

    assert(config_->sst_file_size % (uint64_t)config_->kv_size() == 0);
    size_t kv_pairs = config_->sst_file_size / (uint64_t)config_->kv_size();
    std::string key;
    std::string ts = getCurrentTimestamp();
    std::string value(config_->value_size() - ts.length(), 'V');
    std::string kv_pair;
    std::vector<std::string> keys;

    // write data block
    for (uint32_t i = 0; i < kv_pairs; i++) {
        key = keygen_->Generate();
        keys.emplace_back(key);
        ts = getCurrentTimestamp();
        kv_pair = key + value + ts;
        assert(kv_pair.length() % BLOCK_SIZE == 0);
        ok = Append(fd, kv_pair.c_str(), kv_pair.length());
        if (ok == -1) {
            Close(fd);
            return -1;
        }
    }

    // write index blocks
    uint32_t entries_per_block = BLOCK_SIZE / config_->index_block_entry_size();
    uint32_t index_blocks = kv_pairs / entries_per_block;
    assert(BLOCK_SIZE % config_->index_block_entry_size() == 0);
    assert(kv_pairs % entries_per_block == 0);
    size_t offset = 0;

    for (uint32_t i = 0; i < index_blocks; i++) {
        std::string index_block_entry;
        index_block_entry.reserve(BLOCK_SIZE);

        for (uint32_t j = 0; j < entries_per_block; j++) {
            key = keys.at(i * entries_per_block + j);
            std::string offset_str(sizeof(size_t), '\0');
            std::memcpy(&offset_str[0], &offset, sizeof(size_t));
            std::string mapping = std::string(key.c_str() + offset_str);

            index_block_entry += mapping;
            offset += config_->kv_size();
        }

        assert(index_block_entry.length() == BLOCK_SIZE);
        ok = Append(fd, index_block_entry.c_str(), index_block_entry.length());
        if (ok == -1) {
            Close(fd);
            return -1;
        }
    }

    // write number of entries
    std::string kv_pairs_str(BLOCK_SIZE, '\0');
    std::memcpy(&kv_pairs_str[0], &kv_pairs, sizeof(size_t));
    ok = Append(fd, kv_pairs_str.c_str(), kv_pairs_str.length());
    if (ok == -1) {
        Close(fd);
        return -1;
    }

    struct stat file_info;
    ok = Stat(full_path, &file_info);
    if (ok == -1) {
        Close(fd);
        return -1;
    }

    size_t data_block_size = config_->kv_size() * kv_pairs;
    size_t index_block_size = BLOCK_SIZE * index_blocks;
    assert((uint64_t)file_info.st_size == data_block_size + index_block_size + BLOCK_SIZE);

    printf("Wrote %lu MiB into '%s'\n", file_info.st_size >> 20, name.c_str());
    return Close(fd);
}