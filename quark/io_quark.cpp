#include "io_quark.h"

#include <cassert>
#include <cstdio>

int Init()
{
    int ok;
    ok = quarklibio_client_init();
    if (ok < 0) {
        return -1;
    }

    ok = quarklibio_task_ctx_reg(QUARKSTORE_TASK_MAIN);
    return ok;
}

int AtomGet(atom_id_t* aid)
{
    int ah = quarklibio_atom_get(aid, 0, 0, 0);
    if (ah < 0) {
        fprintf(stderr, "error: quarklibio_atom_get\n");
        return -1;
    }
    return ah;
}

int AtomRelease(int ah)
{
    int ok = quarklibio_atom_release(ah);
    if (ah < 0) {
        fprintf(stderr, "error: quarklibio_atom_release\n");
        return -1;
    }
    return ok;
}

ssize_t AtomRead(int ah, void* buf, size_t len, loff_t off)
{
    assert(off % BLOCK_SIZE == 0);
    int ok = quarklibio_atom_read(ah, buf, len, off, 0);
    if (ok < 0) {
        fprintf(stderr, "error: quarklibio_atom_read\n");
        return -1;
    }
    return ok;
}

ssize_t AtomWrite(int ah, const void* buf, size_t len, loff_t off)
{
    assert(off % BLOCK_SIZE == 0);
    assert((unsigned long) buf % BLOCK_SIZE == 0);
    int ok = quarklibio_atom_write(ah, buf, len, off, 0);
    if (ah < 0) {
        fprintf(stderr, "error: quarklibio_atom_write\n");
        return -1;
    }
    return ok;
}

int AtomRemove(atom_id_t aid)
{
    return 0;
    int ok = quarklibio_atom_remove(aid, 0);
    if (ok < 0) {
        fprintf(stderr, "error: quarklibio_atom_remove\n");
        return -1;
    }
    return ok;
}

int AtomMoveAppend(int dst, int src, size_t len, loff_t off)
{
    int ok = quarklibio_atom_move_append(dst, src, len, off);
    if (ok < 0) {
        fprintf(stderr, "error: quarklibio_atom_move_append\n");
        return -1;
    }

    return ok;
}
