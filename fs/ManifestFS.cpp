#include "ManifestFS.h"

#include <unistd.h>

#include <cstring>

#include "SSTFileManagerFS.h"
#include "io.h"

ManifestFS::ManifestFS(Config* config) : Manifest(config)
{
    manager_ = new SSTFileManagerFS(config);
}

ManifestFS::~ManifestFS()
{
    delete manager_;
}

int ManifestFS::Open()
{
    int fd, ok, current_fd;
    ssize_t rc;
    char manifest_name[BLOCK_SIZE];

    std::string current_path = getFullPath(std::string("CURRENT"));
    current_fd = ::Open(current_path);
    if (current_fd == -1) {
        return -1;
    }

    ok = ::Read(current_fd, manifest_name, BLOCK_SIZE);
    if (ok == -1) {
        return -1;
    }

    std::string full_path = getFullPath(std::string(manifest_name));
    fd = ::Open(full_path);
    if (fd == -1) {
        return -1;
    }

    char buf[BLOCK_SIZE];
    rc = ::Read(fd, buf, BLOCK_SIZE);
    if (rc == -1) {
        goto open_ret;
    }

open_ret:
    return ::Close(fd);
}

int ManifestFS::Populate(uint32_t n)
{
    int ok;
    uint32_t i = 0, id;
    std::lock_guard<std::mutex> lock(mutex_);
    while (i < n) {
        id = sst_ctr_.fetch_add(1);
        ok = manager_->CreateNewSST(id);
        if (ok == -1) {
            continue;
        }
        i = i + 1;
        ok = manager_->PopulateSST(id);
        if (ok == -1) {
            return persist();
        }
        sst_files_.at(0).emplace_back(id);
    }
    return persist();
}

/**
 * @param mutex_ must be held
 */
int ManifestFS::persist()
{
    int id, fd, ok;
    std::string name;
    std::string path;
    while (true) {
        id = manifest_ctr_.fetch_add(1);
        name = "MANIFEST-" + std::to_string(id);
        path = getFullPath(name);

        fd = Create(path);
        if (fd == -1) {
            continue;
        }
        break;
    }

    char buf[BLOCK_SIZE];
    serializeLevels(buf);
    ok = ::Append(fd, std::string(buf), BLOCK_SIZE);
    ::Close(fd);
    if (ok == -1) {
        return -1;
    }

    std::string current_path = getFullPath(std::string("CURRENT"));
    fd = ::OpenForWrite(current_path);
    if (fd == -1) {
        return -1;
    }

    std::memset(buf, 0, BLOCK_SIZE);
    std::memcpy(buf, name.c_str(), name.length());
    return write(fd, buf, BLOCK_SIZE);
}

int ManifestFS::createManifest(int id)
{
    return 0;
}

std::string ManifestFS::getFullPath(std::string name)
{
    return (config_->mode == LOAD ? config_->ddir : config_->wdir) + "/" + name;
}
