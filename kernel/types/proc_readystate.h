#pragma once

#include "../../xstd/xstd_core.h"

typedef enum {
    STATE_RUN,
    STATE_SLEEP,
    STATE_IDLE,
    _STATE_MAX_SENTINEL
} ProcessReadyState;
