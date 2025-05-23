#include "io_fs.h"

#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

int Create(std::string path)
{
    int fd;

    fd = open(path.c_str(), O_RDONLY);
    if (fd != -1) {
        // fprintf(stderr, "file %s exists\n", path.c_str());
        return -1;
    }

    mode_t old = umask(0);
    fd = open(path.c_str(), O_RDWR | O_APPEND | O_DIRECT | O_CREAT, 0666);
    if (fd == -1) {
        perror("Create");
    }
    umask(old);

    return fd;
}

int OpenForWrite(std::string path)
{
    int fd;

    mode_t old = umask(0);
    fd = open(path.c_str(), O_RDWR | O_DIRECT | O_CREAT, 0666);
    if (fd == -1) {
        perror("OpenForWrite");
    }
    umask(old);

    return fd;
}

int Open(std::string path)
{
    int fd;

    mode_t old = umask(0);
    fd = open(path.c_str(), O_RDWR | O_APPEND | O_DIRECT, 0666);
    if (fd == -1) {
        perror("Open");
    }
    umask(old);

    return fd;
}

ssize_t Read(int fd, char* dest, size_t length)
{
    int rc;
    rc = read(fd, dest, length);
    if (rc == -1) {
        perror("Read");
    }
    return rc;
}

off_t Seek(int fd, off_t offset, int whence)
{
    return lseek(fd, offset, whence);
}

ssize_t Write(int fd, void* src, size_t length)
{
    ssize_t wc;
    wc = write(fd, src, length);
    if (wc == -1) {
        perror("write");
    }
    return wc;
}

int Close(int fd)
{
    int ok;
    ok = close(fd);
    if (ok == -1) {
        perror("Close");
        exit(0);
    }
    return ok;
}

int Unlink(std::string path)
{
    return unlink(path.c_str());
}

int Stat(std::string path, struct stat* dest)
{
    int ok;
    ok = stat(path.c_str(), dest);
    if (ok == -1) {
        perror("stat");
    }
    return ok;
}
