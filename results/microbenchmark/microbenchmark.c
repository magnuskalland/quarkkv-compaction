#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <x86intrin.h>
#include <unistd.h>

#include "quarklibio.h"

#define BLOCK_SIZE 4096
#define ITERS 16384

int run_append() 
{
    ssize_t wc;
    int ok, ah1, ah2, ah3, ah4;
    atom_id_t id1 = 0, id2 = 1, id3 = 2, id4 = 3;
    uint64_t ts, delta;
    unsigned int junk;
    char buf[BLOCK_SIZE] = {0};
    
    ok = quarklibio_client_init();
    ok = quarklibio_task_ctx_reg(QUARKSTORE_TASK_MAIN);

    ah1 = quarklibio_atom_get(&id1, 0, 0, 0);
    if (ah1 < 0) {
        return 1;
    }
    ah2 = quarklibio_atom_get(&id2, 0, 0, 0);
    if (ah2 < 0) {
        return 1;
    }
    ah3 = quarklibio_atom_get(&id3, 0, 0, 0);
    if (ah3 < 0) {
        return 1;
    }
    ah4 = quarklibio_atom_get(&id4, 0, 0, 0);
    if (ah4 < 0) {
        return 1;
    }

    for (uint32_t i = 0; i < ITERS; i++) {
        wc = quarklibio_atom_write(ah1, buf, BLOCK_SIZE, BLOCK_SIZE * i, 0);
        if (wc != BLOCK_SIZE) {
            return -1;
        }
        wc = quarklibio_atom_write(ah2, buf, BLOCK_SIZE, BLOCK_SIZE * i, 0);
        if (wc != BLOCK_SIZE) {
            return -1;
        }
    }

    printf("Start perf (%d)\n", getpid());
    sleep(3);

    ts = __rdtscp(&junk);
    for (uint32_t i = 0; i < ITERS / 2; i++) {
        ok = quarklibio_atom_move_append(ah3, ah1, BLOCK_SIZE, BLOCK_SIZE * i);
        if (ok != 0) {
            return -1;
        }
        ok = quarklibio_atom_move_append(ah3, ah2, BLOCK_SIZE, BLOCK_SIZE * i);
        if (ok != 0) {
            return -1;
        }
    }

    for (uint32_t i = ITERS / 2; i < ITERS; i++) {
        ok = quarklibio_atom_move_append(ah4, ah1, BLOCK_SIZE, BLOCK_SIZE * i);
        if (ok != 0) {
            return -1;
        }
        ok = quarklibio_atom_move_append(ah4, ah2, BLOCK_SIZE, BLOCK_SIZE * i);
        if (ok != 0) {
            return -1;
        }
    }
    delta = __rdtscp(&junk);

    printf("Append finished in %ld cycles\n", delta - ts);

    return 0;
}

int run_insert()
{
    ssize_t wc, rc;
    int ok, ah1, ah2;
    atom_id_t id1 = 0, id2 = 1;
    uint64_t ts, delta;
    unsigned int junk;
    char buf[BLOCK_SIZE] = {0};
    
    ok = quarklibio_client_init();
    ok = quarklibio_task_ctx_reg(QUARKSTORE_TASK_MAIN);

    ah1 = quarklibio_atom_get(&id1, 0, 0, 0);
    if (ah1 < 0) {
        return 1;
    }
    ah2 = quarklibio_atom_get(&id2, 0, 0, 0);
    if (ah2 < 0) {
        return 1;
    }

    for (uint32_t i = 0; i < ITERS; i++) {
        wc = quarklibio_atom_write(ah1, buf, BLOCK_SIZE, BLOCK_SIZE * i, 0);
        if (wc != BLOCK_SIZE) {
            return -1;
        }
        wc = quarklibio_atom_write(ah2, buf, BLOCK_SIZE, BLOCK_SIZE * i, 0);
        if (wc != BLOCK_SIZE) {
            return -1;
        }
    }

    printf("Start perf (%d)\n", getpid());
    sleep(3);

    ts = __rdtscp(&junk);
    for (uint32_t i = 0; i < ITERS; i++) {
        ok = quarklibio_atom_move_insert(ah1, BLOCK_SIZE * (i + 1) + BLOCK_SIZE * i, ah2, BLOCK_SIZE * i, BLOCK_SIZE);
        if (ok < 0) {
            return -1;
        }
    }

    for (uint32_t i = 0; i < ITERS; i++) {
        ok = quarklibio_atom_move_insert(ah2, BLOCK_SIZE * i, ah1, (BLOCK_SIZE * i) * 2, BLOCK_SIZE);
        if (ok < 0) {
            return -1;
        }
    }
    delta = __rdtscp(&junk);

    printf("Insert finished in %ld cycles\n", delta - ts);

    return 0;
}

int main(void)
{
    int ok;
    ok = run_insert();
    if (ok < 0) 
    {
        return 1;
    }

    return 0;
}