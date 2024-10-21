
#include "../include/MemTable.h"

#include <map>
#include <string>

MemTable::MemTable(Config* config) : config_(config)
{
    limit_ = config_->sst_file_size / config_->kv_size();
}

KVPair* MemTable::Get(std::string key)
{
    auto it = table_.find(key);
    return it == table_.end() ? nullptr : &it->second;
}

void MemTable::Put(std::string key, std::string val)
{
    KVPair kv(key, config_->value_size());
    table_.insert({key, kv});
}

bool MemTable::Full()
{
    return table_.size() == limit_;
}

std::map<std::string, KVPair>* MemTable::GetTable()
{
    return &table_;
}

void MemTable::Flush()
{
    table_.clear();
}