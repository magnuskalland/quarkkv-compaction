#pragma once
#include <sys/stat.h>

#include <string>

int Create(std::string path);
int OpenForWrite(std::string path);
int Open(std::string path);
ssize_t Read(int fd, char* dest, size_t length);
off_t Seek(int fd, off_t offset, int whence);
ssize_t Write(int fd, void* src, size_t length);
int Close(int fd);
int Unlink(std::string path);
int Stat(std::string path, struct stat* dest);