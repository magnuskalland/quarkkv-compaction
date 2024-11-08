
#include "../DB.h"

#include <cassert>
#include <chrono>
#include <iterator>
#include <thread>

#include "../fs/ManifestFS.h"
#include "../include/DBImpl.h"

DB::DB(Config* config) : db_(config) {}

int DB::Open()
{
    return db_.Open();
}

int DB::Close()
{
    return db_.Close();
}

int DB::Get(std::string key, std::string& dest)
{
    return db_.Get(key, dest);
}

int DB::Put(std::string key, std::string _)
{
    return db_.Put(key, _);
}

std::string DB::ToString()
{
    return db_.ToString();
}

DBImpl::stats DB::GetStats()
{
    return db_.GetStats();
}

void DB::ClearStats()
{
    db_.ClearStats();
}