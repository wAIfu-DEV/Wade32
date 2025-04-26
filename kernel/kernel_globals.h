#pragma once

#include "../xstd/xstd_core.h"

#include "time_type.h"
#include "schedule_event_type.h"

#define KEYBOARD_INPUT_BUFF_SIZE 256

struct _kernel_globals
{
    struct
    {
        u32 tick;
        u32 tickCycles;
        u32 utcCacheTick;
        Time cachedUtcTime;
    } timing;

    struct
    {
        i8 inputBuffer[KEYBOARD_INPUT_BUFF_SIZE];
        u32 inputBufferHead;
    } keyboard;

    struct
    {
        ScheduleEvent eventBuffer[SCHEDULER_BUFF_SIZE];
        u32 eventBufferHead;
        ScheduleEvent timeoutQueue[SCHEDULER_BUFF_SIZE];
        u32 timeoutQueueHead;
        ibool needDefrag;
    } scheduler;

    struct
    {
        VgaInterface vga;
    } screen;
    
    
} kGlobal = {
    .timing = {
        .tick = 0,
        .tickCycles = 0,
        .utcCacheTick = I32_MAXVAL,
        .cachedUtcTime = {0},
    },
    .keyboard = {
        .inputBufferHead = 0,
    },
    .scheduler = {
        .eventBufferHead = 0,
        .timeoutQueueHead = 0,
        .needDefrag = false,
    },
    .screen = {
        .vga = {0},
    }
};

