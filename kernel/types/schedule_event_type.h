#pragma once

#include "../../xstd/xstd_core.h"
#include "../types/extended_tick.h"

#define SCHEDULER_BUFF_SIZE 128

typedef struct _schedule_event
{
    ExTick timeoutTick;
    u32 intervalTick;
    void (*callback)(void* arg0);
    void* arg0;
    ibool isRoutine;
} ScheduleEvent;
