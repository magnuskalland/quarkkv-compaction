#include "ManifestFS.h"

#include "io.h"

int ManifestFS::Open()
{
    int fd, ok;
    ssize_t rc;
    char manifest_name[BLOCK_SIZE] = {0};

    ok = getCurrentManifestPath(manifest_name);
    if (ok == -1) {
        return -1;
    }
    if (ok == 1) {
        return 0;
    }

    std::string full_path = getFullPath(std::string(manifest_name));
    fd = ::Open(full_path);
    if (fd == -1) {
        return -1;
    }

    char buf[BLOCK_SIZE];
    rc = ::Read(fd, buf, BLOCK_SIZE);
    ::Close(fd);
    if (rc == -1) {
        return -1;
    }

    deserializeLevels(buf, BLOCK_SIZE);
    return 0;
}

int ManifestFS::Persist()
{
    int id, fd, ok;
    std::string path;
    char name[BLOCK_SIZE] = {'\0'};

    fd = createNewManifest(name);
    if (fd == -1) {
        return -1;
    }

    char buf[BLOCK_SIZE];
    serializeLevels(buf);

    ok = ::Write(fd, buf, BLOCK_SIZE);
    ::Close(fd);
    if (ok == -1) {
        return -1;
    }

    fd = openCurrent();
    if (fd == -1) {
        return -1;
    }

    ok = ::Seek(fd, 0, SEEK_SET);
    if (ok == -1) {
        return -1;
    }

    std::memset(buf, 0, BLOCK_SIZE);
    std::memcpy(buf, name, BLOCK_SIZE);
    return ::Write(fd, buf, BLOCK_SIZE);
}

std::string ManifestFS::ToString()
{
    std::ostringstream oss;
    char manifest_name[BLOCK_SIZE] = {0};
    int ok;

    ok = getCurrentManifestPath(manifest_name);
    if (ok == -1) {
        return std::string("");
    }

    oss << "Manifest: " << manifest_name << "\n";

    return oss.str();
}

int ManifestFS::openCurrent()
{
    std::string path = getFullPath(std::string("CURRENT"));
    return ::OpenForWrite(path);
}

int ManifestFS::getCurrentManifestPath(char* dest)
{
    int ok;
    int fd;

    fd = openCurrent();
    if (fd == -1) {
        return -1;
    }

    ok = ::Read(fd, dest, BLOCK_SIZE);
    if (ok == -1) {
        return -1;
    }

    ::Close(fd);
    if (ok == 0) {
        ok = createNewManifest(dest);
        if (ok == -1) {
            return -1;
        }
        return 1;
    }

    return 0;
}

int ManifestFS::createNewManifest(char* dest)
{
    int id, fd;
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
        memcpy(dest, name.c_str(), name.length());
        printf("Created new manifest: %s\n", name.c_str());
        return fd;
    }
}