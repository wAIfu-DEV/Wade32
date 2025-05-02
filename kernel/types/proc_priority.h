#pragma once

#include "../../xstd/xstd_core.h"

typedef enum {
    PRIO_HIGHEST,
    PRIO_STANDARD,
    PRIO_LOWEST,
    _PRIO_MAX_SENTINEL
} ProcessPriority;

static u32 PriorityToSliceMs[_PRIO_MAX_SENTINEL] = {
    [PRIO_HIGHEST] = 50,
    [PRIO_STANDARD] = 30,
    [PRIO_LOWEST] = 10,
};
