#include "io_quark.h"  // Includes the function declarations

#include <cassert>  // For assert
#include <cstdio>   // For fprintf

int Init()
{
    return quarklibio_client_init();
}

int AtomGet(atom_id_t aid)
{
    int ah = quarklibio_atom_get(&aid, 0, 0, 0);
    if (ah < 0) {
        fprintf(stderr, "error: quarklibio_atom_get");
        return -1;
    }
    return ah;
}

int AtomRelease(int ah)
{
    int ok = quarklibio_atom_release(ah);
    if (ah < 0) {
        fprintf(stderr, "error: quarklibio_atom_release");
        return -1;
    }
    return ok;
}

ssize_t AtomRead(int ah, void* buf, size_t len, loff_t off)
{
    assert(off % BLOCK_SIZE == 0);
    int ok = quarklibio_atom_read(ah, buf, len, off, 0);
    if (ah < 0) {
        fprintf(stderr, "error: quarklibio_atom_read");
        return -1;
    }
    return ok;
}

ssize_t AtomWrite(int ah, const void* buf, size_t len, loff_t off)
{
    assert(off % BLOCK_SIZE == 0);
    int ok = quarklibio_atom_write(ah, buf, len, off, 0);
    if (ah < 0) {
        fprintf(stderr, "error: quarklibio_atom_write");
        return -1;
    }
    return ok;
}

int AtomRemove(atom_id_t aid)
{
    int ok = quarklibio_atom_remove(aid, 0);
    if (ok < 0) {
        fprintf(stderr, "error: quarklibio_atom_remove");
        return -1;
    }
    return ok;
}

int AtomAppend(int dst, int src, size_t len, loff_t off)
{
    int ok = quarklibio_atom_move_append(dst, src, len, off);
    if (ok < 0) {
        fprintf(stderr, "error: quarklibio_atom_move_append");
        return -1;
    }
    return ok;
}