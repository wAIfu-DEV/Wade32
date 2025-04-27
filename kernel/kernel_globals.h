#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_alloc.h"

#include "extended_tick.h"
#include "time_type.h"
#include "schedule_event_type.h"

#define KERNEL_HEAP_SIZE (4096 * 2)
#define KEYBOARD_INPUT_BUFF_SIZE 256
#define KERNEL_TICK_FREQ 100
#define KERNEL_PRINT_TIME_INTERVAL_MS 100

static struct _kernel_globals
{
    struct
    {
        u32 tickFreq;
        ExTick tick;
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
    
    struct
    {
        volatile i8 heapMem[KERNEL_HEAP_SIZE];
        DebugAllocatorState debugInfo;
        Allocator allocator;
    } heap;    
    
} kGlobal = {
    .timing = {
        .tickFreq = KERNEL_TICK_FREQ,
        .tick = {0},
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
    },
    .heap = {
        .heapMem = {0},
        .allocator = {0},
        .debugInfo = {0},
    },
};

