#pragma once
#include <sys/stat.h>

#include <string>

int Create(std::string path);
int Open(std::string path);
int OpenForWrite(std::string path);
ssize_t Read(int fd, char* dest, size_t length);
off_t Seek(int fd, off_t offset, int whence);
off_t CurrentOffset(int fd);
ssize_t Append(int fd, void* src, size_t length);
ssize_t WriteMeta(int fd, void* src, size_t length);
int Close(int fd);
int Stat(std::string path, struct stat* dest);