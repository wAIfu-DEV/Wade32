#pragma once

#include "../xstd/xstd_core.h"

static u32 randSeed = 1;

u32 pseudorandom(void)
{
    randSeed = randSeed * 1664525 + 1013904223;
    return randSeed;
}

void pseudorandom_set_seed(u32 seed)
{
    randSeed = seed;
}
