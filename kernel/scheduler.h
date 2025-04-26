#pragma once

#include "../xstd/xstd_core.h"

#include "kernel_panic.h"
#include "kernel_errors.h"
#include "kernel_globals.h"

#include "cpu/timer.h"
#include "time.h"

#include "schedule_event_type.h"

void __scheduler_enqueue_timedout(ScheduleEvent ev)
{
    if (kGlobal.scheduler.timeoutQueueHead >= SCHEDULER_BUFF_SIZE)
        kernel_panic(KERR_SCHEDULER_OVERFLOW, "Too many timed out scheduler events.");

    kGlobal.scheduler.timeoutQueue[kGlobal.scheduler.timeoutQueueHead] = ev;
    ++kGlobal.scheduler.timeoutQueueHead;
}

// Checks for timed out events, calls `__scheduler_enqueue_timedout` on timeout
// returns true on events to remove, false on element to keep
ibool __scheduler_handle_timeout(u32 i)
{
    if (kGlobal.scheduler.eventBuffer[i].timeoutTick == U32_MAXVAL)
        return true;

    if (kGlobal.scheduler.eventBuffer[i].timeoutTick >= kGlobal.timing.tick)
        return false;

    ibool ret = false;
    if (kGlobal.scheduler.eventBuffer[i].isRoutine)
    {
        u32 prevTimeout = kGlobal.scheduler.eventBuffer[i].timeoutTick;
        kGlobal.scheduler.eventBuffer[i].timeoutTick = prevTimeout + kGlobal.scheduler.eventBuffer[i].intervalTick;
    }
    else
    {
        kGlobal.scheduler.eventBuffer[i].timeoutTick = U32_MAXVAL;
        ret = true;
    }
    __scheduler_enqueue_timedout(kGlobal.scheduler.eventBuffer[i]);
    return ret;
}

u32 __scheduler_check_timeout_and_queue(void)
{
    u32 ret = 0;
    for (u32 i = 0; i < kGlobal.scheduler.eventBufferHead; ++i)
    {
        if (__scheduler_handle_timeout(i)) ++ret;
    }

    if (ret > 0)
        kGlobal.scheduler.needDefrag = true;
    return ret;
}

void __scheduler_defrag(void)
{
    u32 j = 0;
    for (u32 i = 0; i < kGlobal.scheduler.eventBufferHead; ++i)
    {
        if (kGlobal.scheduler.eventBuffer[i].timeoutTick != U32_MAXVAL)
        {
            if (i != j)
                kGlobal.scheduler.eventBuffer[j] = kGlobal.scheduler.eventBuffer[i];
            ++j;
        }
    }
    kGlobal.scheduler.eventBufferHead = j;
    kGlobal.scheduler.needDefrag = false;
}

/**
 * @brief Processes timed out scheduled events and defragments the scheduler on changes.
 * 
 */
void kernel_scheduler_process(void)
{
    u32 removedEvents = __scheduler_check_timeout_and_queue();

    if (kGlobal.scheduler.needDefrag || removedEvents > 0)
        __scheduler_defrag();

    // events with timeoutTick of U32_MAXVAL (done) are phased out
    // since they are at the end of the array
    kGlobal.scheduler.eventBufferHead = kGlobal.scheduler.eventBufferHead - removedEvents;

    for (u32 i = 0; i < kGlobal.scheduler.timeoutQueueHead; ++i)
    {
        ScheduleEvent ev = kGlobal.scheduler.timeoutQueue[i];
        if (ev.callback != NULL)
            ev.callback(ev.arg0);
    }
    kGlobal.scheduler.timeoutQueueHead = 0;
}

/**
 * @brief Schedules an event or routine.
 * 
 * Routines will be rescheduled on timeout. Use `kernel_unschedule` to remove a scheduled event or routine.
 * 
 * @param timeoutTicks Callback will be called in `timeoutTicks` ticks. Accuracy is not guaranteed.
 * @param callback Callback called by scheduler once event timed out.
 * @param arg0 Argument to pass to callback, use NULL if not args are expected.
 * @param isRoutine If true, will reschedule on timeout.
 */
void kernel_schedule(u32 timeoutTicks, void (*callback)(void* arg0), void* arg0, ibool isRoutine)
{
    if (kGlobal.scheduler.eventBufferHead >= SCHEDULER_BUFF_SIZE)
        kernel_panic(KERR_SCHEDULER_OVERFLOW, "Too many scheduled events/routines.");

    kGlobal.scheduler.eventBuffer[kGlobal.scheduler.eventBufferHead] = (ScheduleEvent){
        .timeoutTick = kGlobal.timing.tick + timeoutTicks,
        .intervalTick = timeoutTicks,
        .arg0 = arg0,
        .callback = callback,
        .isRoutine = isRoutine,
    };
    ++kGlobal.scheduler.eventBufferHead;
}

/**
 * @brief Removes every events or routines with the provided signature.
 * 
 * For example, if 3 events are scheduled with the exact same `timeoutTicks`,
 * `*callback`, `arg0` and `isRoutine` then all those events will be unscheduled.
 * 
 * Example:
 * ```c
 * kernel_schedule(10, my_callback, NULL, false);
 * // This will remove the event
 * kernel_unschedule(10, my_callback, NULL, false);
 * // This won't
 * kernel_unschedule(11, my_callback, NULL, false);
 * ```
 * 
 * @param timeoutTicks 
 * @param callback 
 * @param arg0 
 * @param isRoutine
 */
void kernel_unschedule(u32 timeoutTicks, void (*callback)(void* arg0), void* arg0, ibool isRoutine)
{
    for (u32 i = 0; i < kGlobal.scheduler.eventBufferHead; ++i)
    {
        if (kGlobal.scheduler.eventBuffer[i].callback        == callback
            && kGlobal.scheduler.eventBuffer[i].arg0         == arg0
            && kGlobal.scheduler.eventBuffer[i].intervalTick == timeoutTicks
            && kGlobal.scheduler.eventBuffer[i].isRoutine    == isRoutine)
        {
            kGlobal.scheduler.eventBuffer[i].timeoutTick = U32_MAXVAL;
            kGlobal.scheduler.eventBuffer[i].isRoutine = false;
            kGlobal.scheduler.needDefrag = true;
        }
    }
}


