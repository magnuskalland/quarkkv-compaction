#pragma once
#include <sys/stat.h>

#include <string>

int Create(std::string path);
int Open(std::string path);
int OpenForWrite(std::string path);
ssize_t Read(int fd, char* dest, size_t length);
ssize_t Append(int fd, std::string src, size_t length);
int Close(int fd);
int Stat(std::string path, struct stat* dest);
