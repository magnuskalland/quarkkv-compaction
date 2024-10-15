#include "ManifestFS.h"

#include <unistd.h>

#include <cstring>

#include "SSTManagerFS.h"
#include "io.h"

ManifestFS::ManifestFS(Config* config) : Manifest(config) {}

ManifestFS::~ManifestFS() {}

int ManifestFS::Open()
{
    //     int fd, ok, current_fd;
    //     ssize_t rc;
    //     char manifest_name[BLOCK_SIZE];

    //     std::string current_path = getFullPath(std::string("CURRENT"));
    //     current_fd = ::Open(current_path);
    //     if (current_fd == -1) {
    //         return -1;
    //     }

    //     ok = ::Read(current_fd, manifest_name, BLOCK_SIZE);
    //     if (ok == -1) {
    //         return -1;
    //     }

    //     std::string full_path = getFullPath(std::string(manifest_name));
    //     fd = ::Open(full_path);
    //     if (fd == -1) {
    //         return -1;
    //     }

    //     char buf[BLOCK_SIZE];
    //     rc = ::Read(fd, buf, BLOCK_SIZE);
    //     if (rc == -1) {
    //         goto open_ret;
    //     }

    // open_ret:
    //     return ::Close(fd);
    return 0;
}

/**
 * @param mutex_ must be held
 */
int ManifestFS::Persist()
{
    int id, fd, ok;
    std::string name;
    std::string path;

    while (true) {
        id = ctr_.fetch_add(1);
        name = "MANIFEST-" + std::to_string(id);
        path = getFullPath(name);

        fd = Create(path);
        if (fd == -1) {
            continue;
        }
        break;
    }

    printf("Created new manifest: %s\n", name.c_str());

    char buf[BLOCK_SIZE];
    serializeLevels(buf);

    ok = ::WriteMeta(fd, buf, BLOCK_SIZE);
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
    return WriteMeta(fd, buf, BLOCK_SIZE);
}

std::string ManifestFS::getFullPath(std::string name)
{
    return config_->cwd() + "/" + name;
}
