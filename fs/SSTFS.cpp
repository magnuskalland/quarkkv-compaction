#include "SSTFS.h"

#include <cstring>
#include <iterator>
#include <map>
#include <memory>

#include "../include/KVPair.h"
#include "../include/config.h"
#include "io.h"

SSTFS::SSTFS(Config* config, uint32_t handler, int id, std::string full_name)
    : SST(config, handler, id), full_name_(full_name)
{
}

std::shared_ptr<SST> SSTFS::CreateNewEmpty(Config* config, uint32_t id)
{
    int fd, ok;

    std::string name = createNameFromID(id);
    std::string path = config->cwd() + "/" + name;

    fd = Create(path);
    if (fd == -1) {
        return nullptr;
    }

    std::shared_ptr<SST> sst(new SSTFS(config, fd, id, path));
    return sst;
}

std::unique_ptr<SST> SSTFS::OpenWithID(uint32_t id)
{
    return nullptr;
}

int SSTFS::Persist()
{
    int ok;

    ok = writeIndexBlock();
    if (ok == -1) {
        return -1;
    }

    ok = writeNumberOfEntries();
    if (ok == -1) {
        return -1;
    }

    struct stat info;
    ok = ::Stat(full_name_, &info);
    if (ok == -1) {
        return -1;
    }

    size_t data_block_size = config_->kv_size() * entries_;
    size_t index_block_size = config_->index_block_entry_size() * entries_;
    size_t size_block_size = BLOCK_SIZE;
    assert((uint64_t)info.st_size == data_block_size + index_block_size + size_block_size);
    printf("Persisted SST file of %ld MiBs (data block: %ld MiB, index block: %ld MiB)\n", info.st_size >> 20,
           data_block_size >> 20, index_block_size >> 20);
    return 0;
}

int SSTFS::Append(std::string key)
{
    KVPair kv(key, config_->value_size());
    std::string kv_pair = kv.ToString();
    uint32_t kv_size = config_->kv_size();

    assert(kv_pair.length() == config_->kv_size());
    assert(kv_pair.length() % BLOCK_SIZE == 0);
    int ok = ::Append(handler_, kv_pair, kv_size);
    if (ok == -1) {
        return -1;
    }

    if (entries_ == 0) {
        smallestKey_ = key;
    }

    entries_ = entries_ + 1;
    indexTable_.insert({key, entries_ * kv_size});

    if (indexTable_.size() == config_->sst_file_size / kv_size) {
        largestKey_ = key;
    }

    return 0;
}

int SSTFS::writeIndexBlock()
{
    uint32_t entries_per_block = BLOCK_SIZE / config_->index_block_entry_size();
    uint32_t index_blocks = entries_ / entries_per_block;
    uint32_t index_block_size = index_blocks * BLOCK_SIZE;
    assert(entries_ % entries_per_block == 0);
    size_t offset = 0;
    char buf[index_block_size];

    std::map<std::string, uint32_t>::iterator it;
    for (it = indexTable_.begin(); it != indexTable_.end(); it++) {
        std::string indexed_offset(sizeof(size_t), '\0');
        std::memcpy(&indexed_offset[0], &it->second, sizeof(uint32_t));
        std::string mapping(it->first + indexed_offset);
        assert(mapping.length() == config_->index_block_entry_size());
        memcpy(&buf[offset], mapping.c_str(), mapping.length());
        offset = offset + config_->index_block_entry_size();
    }

    assert(offset == index_block_size);
    int ok = ::Append(handler_, buf, index_block_size);
    if (ok == -1) {
        return -1;
    }

    return 0;
}

int SSTFS::writeNumberOfEntries()
{
    std::string number_of_kv_pairs(BLOCK_SIZE, '\0');
    std::memcpy(&number_of_kv_pairs[0], &entries_, sizeof(uint32_t));
    return ::Append(handler_, number_of_kv_pairs.c_str(), BLOCK_SIZE);
}