
#include "../include/MemTable.h"

#include <map>
#include <string>

MemTable::MemTable(size_t limit) : limit_(limit) {}

std::string* MemTable::Get(std::string key)
{
    auto it = table_.find(key);
    return it == table_.end() ? nullptr : &it->second;
}

void MemTable::Put(std::string key, std::string val)
{
    table_[key] = val;
}

bool MemTable::Full()
{
    return table_.size() == limit_;
}

std::map<std::string, std::string>* MemTable::GetTable()
{
    return &table_;
}

void MemTable::Clear()
{
    table_.clear();
}