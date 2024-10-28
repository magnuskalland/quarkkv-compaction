#pragma once

#include <sys/types.h>

#include <cstdint>

extern "C" {
#include "quarklibio.h"
}

#define BLOCK_SIZE 4096

// Function declarations
ssize_t Read(int ah, void *buf, size_t len, loff_t off);
ssize_t Write(int ah, const void *buf, size_t len, loff_t off);
int InsertBlock(int ah_dst, loff_t off_dst, int ah_src, loff_t off_src, size_t len);
int RemoveBlock(int ah, loff_t off, size_t len);
int GetAtom(atom_id_t aid);
