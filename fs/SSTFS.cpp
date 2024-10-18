#include "SSTFS.h"

#include <unistd.h>

#include <cstring>
#include <iterator>
#include <map>
#include <memory>

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

std::shared_ptr<SST> SSTFS::OpenWithID(Config* config, uint32_t id)
{
    int fd, ok;

    std::string name = createNameFromID(id);
    std::string path = config->cwd() + "/" + name;

    fd = Open(path);
    if (fd == -1) {
        return nullptr;
    }

    std::shared_ptr<SST> sst(new SSTFS(config, fd, id, path));
    sst->Parse();
    return sst;
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
    printf("Persisted %s with %ld MiBs (data block: %ld MiB, index block: %ld MiB)\n", createNameFromID(id_).c_str(),
           info.st_size >> 20, data_block_size >> 20, index_block_size >> 20);
    return 0;
}

int SSTFS::Parse()
{
    int ok;

    ok = readNumberOfEntries();
    if (ok == -1) {
        return -1;
    }

    printf("Entries in SST %2d is %d\n", id_, entries_);

    ok = readIndexBlock();
    if (ok == -1) {
        return -1;
    }

    return 0;
}

int SSTFS::Close()
{
    return ::Close(handler_);
}

int SSTFS::Get(std::string key, KVPair** dest)
{
    if (key > largestKey_ || key < smallestKey_) {
        *dest = nullptr;
        return 0;
    }
    auto it = indexTable_.find(key);
    if (it == indexTable_.end()) {
        *dest = nullptr;
        return -1;
    }

    return readKVAtOffset(it->second, dest);
}

int SSTFS::Append(std::string key)
{
    KVPair kv(key, config_->value_size());
    char kv_pair[config_->kv_size()];
    memcpy(kv_pair, kv.ToString().c_str(), config_->kv_size());
    uint32_t kv_size = config_->kv_size();

    int ok = ::Append(handler_, kv_pair, kv_size);
    if (ok == -1) {
        return -1;
    }

    if (entries_ == 0) {
        smallestKey_ = key;
    }

    indexTable_.insert({key, entries_ * kv_size});
    entries_ = entries_ + 1;

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
    std::map<std::string, uint64_t>::iterator it;
    for (it = indexTable_.begin(); it != indexTable_.end(); it++) {
        assert(it->first.size() == config_->key_size);
        memcpy(&buf[offset], it->first.c_str(), config_->key_size);
        memcpy(&buf[offset + config_->key_size], &it->second, sizeof(uint64_t));
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
    char number_of_kv_pairs[BLOCK_SIZE] = {'\0'};
    std::memcpy(&number_of_kv_pairs[0], &entries_, sizeof(uint32_t));
    printf("Writing number of KV pairs: %d\n", entries_);
    return ::Append(handler_, number_of_kv_pairs, BLOCK_SIZE);
}

int SSTFS::readIndexBlock()
{
    int ok;
    size_t offset = 0;
    uint64_t index_block_offset = entries_ * config_->kv_size();

    ok = Seek(handler_, index_block_offset, SEEK_SET);
    if (ok == -1) {
        return -1;
    }

    uint32_t entries_per_block = BLOCK_SIZE / config_->index_block_entry_size();
    uint32_t index_blocks = entries_ / entries_per_block;
    uint32_t index_block_size = index_blocks * BLOCK_SIZE;

    char buf[index_block_size];
    ok = ::Read(handler_, buf, index_block_size);
    if (ok == -1) {
        return -1;
    }

    char key[config_->key_size];
    uint64_t val;
    for (uint32_t i = 0; i < entries_; i++) {
        memcpy(key, &buf[offset], config_->key_size);
        memcpy(&val, &buf[offset + config_->key_size], sizeof(uint64_t));
        indexTable_.insert({std::string(key, config_->key_size), val});
        offset = offset + config_->index_block_entry_size();
    }

    assert(indexTable_.size() == entries_);

    smallestKey_ = indexTable_.begin()->first;
    largestKey_ = (--indexTable_.end())->first;

    return 0;
}

int SSTFS::readNumberOfEntries()
{
    int ok;
    char buf[BLOCK_SIZE] = {0};

    ok = Seek(handler_, -BLOCK_SIZE, SEEK_END);
    if (ok == -1) {
        return -1;
    }

    ok = Read(handler_, buf, BLOCK_SIZE);
    if (ok == -1) {
        return -1;
    }

    std::memcpy(&entries_, buf, sizeof(entries_));
    return 0;
}

int SSTFS::readKVAtOffset(uint64_t off, KVPair** kv_pair)
{
    int ok;

    assert(off % BLOCK_SIZE == 0);

    ok = Seek(handler_, off, SEEK_SET);
    if (ok == -1) {
        return -1;
    }

    uint32_t kv_size = config_->kv_size();
    char buf[kv_size];
    ok = ::Read(handler_, buf, kv_size);
    if (ok == -1) {
        return -1;
    }

    std::string key(buf, config_->key_size);
    std::string ts(&buf[config_->kv_size() - config_->ts_size], config_->ts_size);

    *kv_pair = new KVPair(key, ts, config_->value_size());
    return 0;
}