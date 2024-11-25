#include "../include/SST.h"

#include <cstring>

/* Public */

SST::~SST() {}

int SST::Persist(uint64_t ts)
{
    int ok;

    largestKey_ = indexTable_.rbegin()->first;

    ok = writeIndexBlock();
    if (ok == -1) {
        return -1;
    }

    ok = writeNumberOfEntries();
    if (ok == -1) {
        return -1;
    }

    persisted_ = true;
    ts_ = ts;

    return 0;
}

int SST::Parse()
{
    int ok;

    uint64_t entries = config_->sst_file_size / config_->kv_size();
    indexBlockSize_ = config_->index_block_entry_size() * entries;
    dataBlockOffset_ = 0;
    indexBlockOffset_ = config_->sst_file_size;
    numberOfEntriesOffset_ = indexBlockOffset_ + indexBlockSize_;

    ok = readNumberOfEntries();
    if (ok == -1) {
        return -1;
    }

    ok = readIndexBlock();
    if (ok == -1) {
        return -1;
    }

    largestKey_ = indexTable_.rbegin()->first;
    persisted_ = true;
    return 0;
}

int SST::Lookup(std::string key, KVPair** dest)
{
    if (key > largestKey_ || key < smallestKey_) {
        *dest = nullptr;
        return 0;
    }
    auto it = indexTable_.find(key);
    if (it == indexTable_.end()) {
        *dest = nullptr;
        return 0;
    }

    return readKV(it->second, dest);
}

int SST::AddKey(std::string key, uint64_t ts)
{
    KVPair kv(key, ts, config_->value_size());
    return AddKV(&kv);
}

int SST::AddKV(KVPair* kv)
{
    int ok;
    ok = appendKV(kv);
    if (ok == -1) {
        return -1;
    }

    if (entries_ == 0) {
        smallestKey_ = kv->GetKey();
    }

    indexTable_.insert({kv->GetKey(), entries_ * config_->kv_size()});
    entries_ = entries_ + 1;

    return 0;
}

int SST::GetKVAtIndex(uint32_t index, KVPair** dest)
{
    int ok;
    assert(index >= 0 && index < entries_);

    ok = readKV((off_t)(index * config_->kv_size()), dest);
    if (ok == -1) {
        return -1;
    }

    return 0;
}

int SST::SearchClosest(std::string key)
{
    auto match = indexTable_.lower_bound(key);
    if (match != indexTable_.end()) {
        return (int)match->second / config_->kv_size();
    }
    return -1;
}

bool SST::operator>(const SST& other) const
{
    return smallestKey_ > other.smallestKey_;
}

int SST::GetHandler()
{
    return handler_;
}

uint32_t SST::GetEntries()
{
    return entries_;
}

void SST::SetLevel(uint32_t level)
{
    level_ = level;
}

uint32_t SST::GetLevel()
{
    return level_;
}

std::string SST::GetSmallestKey()
{
    return smallestKey_;
}

std::string SST::GetLargestKey()
{
    return largestKey_;
}

int SST::GetID()
{
    return id_;
}

std::string SST::GetName()
{
    return createNameFromID(id_);
}

bool SST::IsMarkedForCompaction()
{
    return markedForCompaction_;
}

void SST::MarkForCompaction()
{
    assert(!markedForCompaction_);
    markedForCompaction_ = true;
}

void SST::UnmarkForCompaction()
{
    assert(markedForCompaction_);
    markedForCompaction_ = false;
}

bool SST::IsFull()
{
    return entries_ == config_->sst_file_size / config_->kv_size();
}

bool SST::IsPersisted()
{
    return persisted_;
}

uint64_t SST::GetPersistTime()
{
    return ts_;
}

/* Protected */

SST::SST(Config* config, uint32_t handler, int id)
    : config_(config), handler_(handler), id_(id)
{
    entries_ = 0;
    smallestKey_ = "";
    largestKey_ = "";
}

int SST::appendKV(KVPair* kv)
{
    uint32_t kv_size = config_->kv_size();
    char kv_pair[kv_size];
    memcpy(kv_pair, kv->ToBinary().c_str(), kv_size);

    int ok = append(kv_pair, kv_size);
    if (ok == -1) {
        return -1;
    }
    return 0;
}

int SST::writeIndexBlock()
{
    uint32_t entries_per_block = BLOCK_SIZE / config_->index_block_entry_size();
    uint32_t index_blocks = (entries_ % entries_per_block == 0)
                                ? entries_ / entries_per_block
                                : entries_ / entries_per_block + 1;
    uint32_t index_block_size = index_blocks * BLOCK_SIZE;
    // assert(entries_ % entries_per_block == 0);
    size_t offset = 0;
    char buf[index_block_size];

    for (auto it = indexTable_.begin(); it != indexTable_.end(); it++) {
        assert(it->first.size() == config_->key_size);
        memcpy(&buf[offset], it->first.c_str(), config_->key_size);
        memcpy(&buf[offset + config_->key_size], &it->second, sizeof(uint64_t));
        offset = offset + config_->index_block_entry_size();
    }

    // assert(offset == index_block_size);
    int ok = append(buf, index_block_size);
    if (ok == -1) {
        return -1;
    }

    indexBlockOffset_ = entries_ * config_->kv_size();
    indexBlockSize_ = index_block_size;
    return 0;
}

int SST::writeNumberOfEntries()
{
    int ok;
    char number_of_kv_pairs[BLOCK_SIZE] = {'\0'};

    assert(indexBlockOffset_ != -1);
    assert((int)indexBlockSize_ != -1);

    std::memcpy(&number_of_kv_pairs[0], &entries_, sizeof(uint32_t));
    ok = append(number_of_kv_pairs, BLOCK_SIZE);
    if (ok == -1) {
        return -1;
    }

    numberOfEntriesOffset_ = indexBlockOffset_ + indexBlockSize_;
    return 0;
}

int SST::readKV(off_t offset, KVPair** dest)
{
    int ok;

    assert(offset % BLOCK_SIZE == 0);

    uint32_t kv_size = config_->kv_size();
    char buf[kv_size];
    ok = read(buf, kv_size, offset);
    if (ok == -1) {
        return -1;
    }

    std::string key(buf, config_->key_size);
    size_t ts_index = config_->kv_size() - (config_->ts_size - 2);
    char hex_str[17] = {0};

    std::strncpy(hex_str, buf + ts_index, 16);
    uint64_t ts = std::stoull(hex_str, nullptr, 16);

    *dest = new KVPair(key, ts, config_->value_size(), handler_, offset);
    return 0;
}

int SST::readIndexBlock()
{
    int ok;
    size_t offset = 0;

    assert(indexBlockOffset_ != -1);
    assert((int)indexBlockSize_ != -1);

    char buf[indexBlockSize_];
    ok = read(buf, indexBlockSize_, indexBlockOffset_);
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

int SST::readNumberOfEntries()
{
    int ok;
    char buf[BLOCK_SIZE] = {0};

    ok = read(buf, BLOCK_SIZE, numberOfEntriesOffset_);
    if (ok == -1) {
        return -1;
    }

    std::memcpy(&entries_, buf, sizeof(entries_));
    return 0;
}

std::string SST::createNameFromID(uint32_t id)
{
    std::ostringstream oss;
    oss << std::setw(6) << std::setfill('0') << id << ".sst";
    return oss.str();
}