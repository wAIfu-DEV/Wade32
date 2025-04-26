#pragma once

#include "../xstd/xstd_core.h"

#define SCHEDULER_BUFF_SIZE 256

typedef struct _schedule_event
{
    u32 timeoutTick;
    u32 intervalTick;
    void (*callback)(void* arg0);
    void* arg0;
    ibool isRoutine;
} ScheduleEvent;
