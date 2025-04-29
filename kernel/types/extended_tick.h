#pragma once

#include "../../xstd/xstd_core.h"

typedef struct
{
    u32 tick;
    u32 cycle;
} ExTick;

void extick_add(ExTick* a, const ExTick b)
{
    u32 res = a->tick + b.tick;

    if (res < a->tick || res < b.tick)
    {
        ++a->cycle;
    }
    a->tick = res;
}

void extick_add_ticks(ExTick* a, const u32 b)
{
    u32 res = a->tick + b;

    if (res < a->tick || res < b)
    {
        ++a->cycle;
    }
    a->tick = res;
}

void extick_sub(ExTick* a, const ExTick b)
{
    u32 res = a->tick - b.tick;

    if (res > a->tick || res > b.tick)
    {
        --a->cycle;
    }
    a->tick = res;
}

ibool extick_greater_than(const ExTick a, const ExTick b)
{
    if (a.cycle > b.cycle) return true;
    if (a.cycle == b.cycle && a.tick > b.tick) return true;
    return false;
}

ibool extick_lesser_than(const ExTick a, const ExTick b)
{
    if (a.cycle < b.cycle) return true;
    if (a.cycle == b.cycle && a.tick < b.tick) return true;
    return false;
}
