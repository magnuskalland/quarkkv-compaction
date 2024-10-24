#include "SSTFS.h"

#include <unistd.h>

#include <cstring>
#include <iterator>
#include <map>
#include <memory>

#include "../include/config.h"
#include "io.h"

SSTFS::SSTFS(Config* config, uint32_t handler, int id, std::string full_name)
    : SST(config, handler, id), fullName_(full_name)
{
    entries_ = 0;
    smallestKey_ = "";
    largestKey_ = "";
}

SSTFS::~SSTFS() {}

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

int SSTFS::Remove()
{
    int ok;

    ok = ::Close(handler_);
    if (ok == -1) {
        return -1;
    }

    ok = ::Unlink(fullName_);
    if (ok == -1) {
        return -1;
    }
    return 0;
}

std::string SSTFS::GetName()
{
    return createNameFromID(id_);
}

int SSTFS::append(char* buf, size_t size)
{
    int ok;

    ok = ::Seek(handler_, 0, SEEK_END);
    if (ok == -1) {
        return -1;
    }

    ok = ::Write(handler_, buf, size);
    if (ok == -1) {
        return -1;
    }

    return ok;
}

int SSTFS::read(char* buf, size_t size, off_t offset)
{
    int ok;

    ok = ::Seek(handler_, offset, SEEK_SET);
    if (ok == -1) {
        return -1;
    }

    ok = ::Read(handler_, buf, size);
    if (ok == -1) {
        return -1;
    }

    return ok;
}

// bool SSTFS::verifyPersisted()
// {
//     struct stat info;
//     int ok = ::Stat(full_name_, &info);
//     if (ok == -1) {
//         return -1;
//     }

//     size_t data_block_size = config_->kv_size() * entries_;
//     size_t index_block_size = config_->index_block_entry_size() * entries_;
//     size_t size_block_size = BLOCK_SIZE;
//     printf("Persisted %s with %ld MiBs (data block: %ld MiB, index block: %ld MiB)\n",
//     createNameFromID(id_).c_str(),
//            info.st_size >> 20, data_block_size >> 20, index_block_size >> 20);
//     return (uint64_t)info.st_size == data_block_size + index_block_size +
//     size_block_size;
// }
