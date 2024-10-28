#include "io_quark.h"  // Includes the function declarations

#include <cassert>  // For assert
#include <cstdio>   // For fprintf

int GetAtom(atom_id_t aid)
{
    int ah = quarklibio_atom_get(&aid, 0, 0, 0);
    if (ah == -1) {
        fprintf(stderr, "error: quarklibio_atom_get");
        return -1;
    }
    return ah;
}

ssize_t Read(int ah, void* buf, size_t len, loff_t off)
{
    assert(off % BLOCK_SIZE == 0);
    int ok = quarklibio_atom_read(ah, buf, len, off, 0);
    if (ok == -1) {
        fprintf(stderr, "error: quarklibio_atom_read");
        return -1;
    }
    return ok;
}

ssize_t Write(int ah, const void* buf, size_t len, loff_t off)
{
    assert(off % BLOCK_SIZE == 0);
    int ok = quarklibio_atom_write(ah, buf, len, off, 0);
    if (ok == -1) {
        fprintf(stderr, "error: quarklibio_atom_write");
        return -1;
    }
    return ok;
}

int InsertBlock(int ah_dst, loff_t off_dst, int ah_src, loff_t off_src, size_t len)
{
    assert(off_dst % BLOCK_SIZE == 0);
    assert(off_src % BLOCK_SIZE == 0);
    assert(len == BLOCK_SIZE);
    int ok = quarklibio_atom_insert_block(ah_dst, off_dst, ah_src, off_src, len);
    if (ok == -1) {
        fprintf(stderr, "error: quarklibio_atom_insert_block");
        return -1;
    }
    return ok;
}

int RemoveBlock(int ah, loff_t off, size_t len)
{
    assert(off % BLOCK_SIZE == 0);
    assert(len == BLOCK_SIZE);
    int ok = quarklibio_atom_remove_block(ah, off, len);
    if (ok == -1) {
        fprintf(stderr, "error: quarklibio_atom_remove_block");
        return -1;
    }
    return ok;
}
