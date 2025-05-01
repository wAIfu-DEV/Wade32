#pragma once

#include "../../xstd/xstd_core.h"
#include "../random.h"

typedef struct
{
    u32 id1;
    u32 id2;
} ExID;

ExID id_generate_unsafe()
{
    return (ExID){
        .id1 = pseudorandom(),
        .id2 = pseudorandom(),
    };
}

ibool id_equals(const ExID a, const ExID b)
{
    return (a.id1 == b.id1 && a.id2 == b.id2);
}
