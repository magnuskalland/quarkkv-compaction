
#include "../include/DBImpl.h"

#include <cassert>
#include <chrono>
#include <iterator>
#include <thread>

#include "../fs/CompacterFS.h"
#include "../fs/ManifestFS.h"
#include "../include/CompactionIterator.h"
#include "../quark/CompacterQuark.h"
#include "../quark/CopyCompacterQuark.h"
#include "../quark/ManifestQuark.h"

DBImpl::DBImpl(Config* config) : config_(config)
{
    manager_ = new SSTManager(config);
    memTable_ = new MemTable(config);

    if (config->engine == FS) {
        manifest_ = new ManifestFS(config);
        compacter_ = new CompacterFS(config, manager_, &ssts_);
    }
    else if (config->engine == QUARKSTORE) {
        manifest_ = new ManifestQuark(config);
        compacter_ = new CopyCompacterQuark(config, manager_, &ssts_);
    }
    else if (config->engine == QUARKSTORE_APPEND) {
        manifest_ = new ManifestQuark(config);
        compacter_ = new CompacterQuark(config, manager_, &ssts_);
    }

    for (uint32_t level = 0; level < config->n_levels; level++) {
        ssts_.emplace_back(std::set<std::shared_ptr<SST>, SST::SSTComparator>());
    }
};

int DBImpl::Open()
{
    int ok;
    ok = manifest_->Open();
    if (ok != 0) {
        return -1;
    }

    std::vector<std::vector<int>> ssts = manifest_->GetSSTs();
    for (uint32_t i = 0; i < ssts.size(); i++) {
        std::vector<int> level = ssts.at(i);
        for (auto it = level.begin(); it != level.end(); it++) {
            std::shared_ptr<SST> sst = manager_->ReadSST(*it);
            if (sst == nullptr) {
                return -1;
            }
            sst.get()->SetLevel(i);
            ssts_.at(i).insert(sst);
        }
    }

    printf("%s\n", ToString().c_str());

    return 0;
}

int DBImpl::Close()
{
    int ok;

    ok = manifest_->Persist();
    if (ok == -1) {
        return -1;
    }

    manifest_->~Manifest();

    return 0;
}

int DBImpl::Get(std::string key, std::string& dest)
{
    int ok;
    KVPair* kv = nullptr;

    padKey(key);

    // search in memtable
    kv = memTable_->Get(key);
    if (kv) {
        dest = kv->ToString();
        return 0;
    }

    KVPair* latest = nullptr;
    SST* sst;
    std::set<std::shared_ptr<SST>, SST::SSTComparator> level0 = ssts_.at(0);

    // search in level 0
    for (auto it = level0.begin(); it != level0.end(); ++it) {
        sst = (*it).get();
        ok = manager_->Get(sst, key, &kv);
        if (ok == -1) {
            return -1;
        }

        if (kv && (!latest || kv > latest)) {
            latest = kv;
        }
    }

    if (latest) {
        dest = latest->ToString();
        return 0;
    }

    // search in rest of levels
    std::vector<std::set<std::shared_ptr<SST>, SST::SSTComparator>>::iterator outer =
        ssts_.begin();
    outer++;
    int level = 1;
    for (; outer != ssts_.end(); ++outer) {
        for (auto it = outer->begin(); it != outer->end(); ++it) {
            sst = (*it).get();
            ok = manager_->Get(sst, key, &kv);
            if (ok == -1) {
                return -1;
            }

            if (kv) {
                dest = kv->ToString();
                return 0;
            }
        }
        level++;
    }

    return 0;
}

int DBImpl::Put(std::string key, std::string _)
{
    int ok;

    padKey(key);

    memTable_->Put(key, manager_->ClockGetAndIncrement(), _);
    if (memTable_->Full()) {
        ok = flush();
        if (ok == -1) {
            return -1;
        }
    }

    return 0;
}

int DBImpl::Scan(std::string start, int len,
                 std::map<std::string, std::string>::iterator& dest)
{
    padKey(start);
    int ok;
    std::priority_queue<KVPair, std::vector<KVPair>, KVPair::KVPairComparator> pairs;
    std::string largest;

    // scan memtable
    auto table = memTable_->GetTable();
    for (auto it = table->lower_bound(start); it != table->end(); it++) {
        pairs.emplace((*it).second);

        if ((int)pairs.size() == len) {
            break;
        }
    }

    if (pairs.size() > 0) {
        largest = pairs.top().GetKey();
    }

    // scan all levels
    int l = 0;
    for (auto level = ssts_.begin(); level != ssts_.end(); level++) {
        if ((*level).size() == 0) {
            continue;
        }

        LevelIterator it(config_, *level);
        ok = it.Seek(start);
        if (ok == -1) {
            return -1;
        }

        while (it.Get() != it.End()) {
            auto e = it.Get();
            it.Next();
            if (pairs.size() < (size_t)len) {
                pairs.emplace(*e);
                continue;
            }

            if (*e > pairs.top()) {
                break;
            }

            pairs.pop();
            pairs.push(*e);
        }
    }
    return 0;
}

std::string DBImpl::ToString()
{
    int ok;
    std::ostringstream oss;
    float avgSize = 0.0;
    uint32_t ssts = 0;

    oss << "Total compaction stats:\n" << compacter_->totalStats.ToString() << "\n";

    oss << manifest_->ToString();
    for (uint32_t i = 0; i < ssts_.size(); i++) {
        for (auto it = ssts_.at(i).begin(); it != ssts_.at(i).end(); it++) {
            ssts++;
            avgSize += (*it).get()->GetEntries();
        }
    }

    avgSize = avgSize == 0.0 ? 0.0 : (avgSize * config_->kv_size()) / ssts;

    oss << "Number of live SSTs: " << ssts << "\n";
    oss << "Average live SST size: " << ((uint32_t)avgSize >> 20) << " MiB"
        << "\n";
    oss << "Database size: " << ((uint64_t)(avgSize * ssts) >> 20) << " MiB"
        << "\n";

    for (uint32_t i = 0; i < ssts_.size(); i++) {
        oss << "Level " << i << ": ";
        for (auto it = ssts_.at(i).begin(); it != ssts_.at(i).end(); it++) {
            oss << (*it).get()->GetName() << " ";
        }
        oss << "(" << ssts_.at(i).size() << "/" << config_->maxSizeOfLevel(i) << ")"
            << "\n";
    }

    return oss.str();
}

void DBImpl::ClearStats()
{
    stats_.compactionTimes.Clear();
}

DBImpl::stats DBImpl::GetStats()
{
    return stats_;
}

int DBImpl::populate(int n)
{
    printf("Populating DBImpl with %d entries\n", n);
    std::shared_ptr<SST> sst;
    int ok;

    for (int i = 0; i < n; i++) {
        sst = manager_->NewEmptySST();
        if (sst == nullptr) {
            return -1;
        }

        ok = manager_->PopulateSST(*sst);
        if (ok == -1) {
            return -1;
        }

        ssts_.at(0).insert(sst);
        manifest_->AddToLevel(i, sst.get()->GetID());
    }

    ok = manifest_->Persist();
    return 0;
}

int DBImpl::flush()
{
    std::shared_ptr<SST> sst;
    int ok;

    ok = manager_->FlushToSST(memTable_, sst);
    if (ok == -1) {
        return -1;
    }

    memTable_->Flush();

    manifest_->AddToLevel(0, sst.get()->GetID());
    ssts_.at(0).insert(sst);

    if (ssts_.at(0).size() > config_->level0_max_size) {
        ok = compact();
    }

    flushes_++;
    return ok;
}

int DBImpl::compact()
{
    int ok;

    auto compactionStart = TIME_NOW;
    ok = compacter_->Compact();
    auto compactionEnd = TIME_DURATION(compactionStart, TIME_NOW);
    stats_.compactionTimes.Insert(compactionEnd);

    if (ok == -1) {
        fprintf(stderr, "error during compaction\n");
        return -1;
    }

    manifest_->Update(ssts_);
    ok = manifest_->Persist();
    if (ok == -1) {
        return -1;
    }

    printf("%s\n", ToString().c_str());
    return 0;
}

void DBImpl::padKey(std::string& key)
{
    key.append(std::string(config_->key_size - key.size(), '_'));
}
