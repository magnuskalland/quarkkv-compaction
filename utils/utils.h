#pragma once

#include <stdint.h>

inline int round_up_to_nearest_multiple(uint64_t from, uint64_t to)
{
    return ((from + to - 1) / to) * to;
}