#include "../include/Manifest.h"

#include <unistd.h>

#include <cstring>
#include <set>

#include "../include/SSTManager.h"

Manifest::Manifest(Config* config, int ctr) : config_(config), ctr_(ctr)
{
    for (uint32_t level = 0; level < config_->n_levels; level++) {
        sst_files_.emplace_back(std::vector<int>());
    }
}

/**
 * Get IDs of SST files of all levels.
 */
std::vector<std::vector<int>> Manifest::GetSSTs()
{
    return sst_files_;
}

/**
 * Add ID to level.
 */
void Manifest::AddToLevel(int level, int id)
{
    sst_files_.at(level).emplace_back(id);
}

void Manifest::Update(
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>> ssts)
{
    sst_files_.clear();
    for (auto outer = ssts.begin(); outer != ssts.end(); outer++) {
        std::vector<int> set;
        for (auto inner = (*outer).begin(); inner != (*outer).end(); inner++) {
            set.emplace_back((*inner).get()->GetID());
        }
        sst_files_.emplace_back(set);
    }
}

std::vector<int> Manifest::FlattenSSTs()
{
    return std::accumulate(sst_files_.begin(), sst_files_.end(), std::vector<int>(),
                           [](std::vector<int>& acc, const std::vector<int>& vec) {
                               acc.insert(acc.end(), vec.begin(), vec.end());
                               return acc;
                           });
}

void Manifest::deserializeLevels(char* buf, size_t bufsize)
{
    uint32_t offset = 0;
    uint32_t levels;
    memcpy(&levels, &buf[offset], sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for (uint32_t level = 0; level < levels; level++) {
        uint32_t entries;
        memcpy(&entries, &buf[offset], sizeof(uint32_t));
        offset += sizeof(uint32_t);

        for (uint32_t j = 0; j < entries; j++) {
            uint32_t id;
            memcpy(&id, &buf[offset], sizeof(uint32_t));
            sst_files_.at(level).emplace_back(id);
            offset += sizeof(uint32_t);
        }
    }
}

void Manifest::serializeLevels(char* buf)
{
    uint32_t offset = 0;
    std::memset(buf, 0, BLOCK_SIZE);
    ssize_t n_levels = sst_files_.size();
    std::memcpy(&buf[offset], (void*)&n_levels, sizeof(uint32_t));
    offset = offset + sizeof(uint32_t);

    for (uint32_t level = 0; level < sst_files_.size(); level++) {
        uint32_t number = sst_files_.at(level).size();
        std::memcpy(&buf[offset], (void*)&number, sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);

        for (uint32_t entry = 0; entry < sst_files_.at(level).size(); entry++) {
            uint32_t number = sst_files_.at(level).at(entry);
            std::memcpy(&buf[offset], (void*)&number, sizeof(uint32_t));
            offset = offset + sizeof(uint32_t);
        }
    }

    assert(offset == sizeof(uint32_t) + n_levels * sizeof(uint32_t) +
                         getNumberOfLiveSSTs() * sizeof(uint32_t));
}

uint32_t Manifest::getNumberOfLiveSSTs()
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < sst_files_.size(); i++) {
        count = count + sst_files_.at(i).size();
    }
    return count;
}

std::string Manifest::getFullPath(std::string name)
{
    return config_->fs_dbdir + "/" + name;
}