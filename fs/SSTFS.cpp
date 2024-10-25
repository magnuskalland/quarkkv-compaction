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
