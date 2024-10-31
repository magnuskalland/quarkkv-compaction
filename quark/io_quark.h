#pragma once

#include <sys/types.h>

#include <cstdint>

extern "C" {
#include "quarklibio.h"
}

#define BLOCK_SIZE 4096

int Init();
int AtomGet(atom_id_t *aid);
int AtomRelease(int ah);
ssize_t AtomRead(int ah, void *buf, size_t len, loff_t off);
ssize_t AtomWrite(int ah, const void *buf, size_t len, loff_t off);
int AtomRemove(atom_id_t aid);
int AtomMoveAppend(int dst, int src, size_t len, loff_t off);