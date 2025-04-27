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
    if (kGlobal.scheduler.eventBuffer[i].timeoutTick.cycle == U32_MAXVAL)
        return true;

    if (extick_greater_than(kGlobal.scheduler.eventBuffer[i].timeoutTick, kGlobal.timing.tick))
        return false;

    ibool ret = false;
    if (kGlobal.scheduler.eventBuffer[i].isRoutine)
    {
        ExTick prevTimeout = kGlobal.scheduler.eventBuffer[i].timeoutTick;
        extick_add_ticks(&prevTimeout, kGlobal.scheduler.eventBuffer[i].intervalTick);
        kGlobal.scheduler.eventBuffer[i].timeoutTick = prevTimeout;
    }
    else
    {
        kGlobal.scheduler.eventBuffer[i].timeoutTick.cycle = U32_MAXVAL;
        kGlobal.scheduler.eventBuffer[i].timeoutTick.tick = 0;
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
        if (kGlobal.scheduler.eventBuffer[i].timeoutTick.cycle != U32_MAXVAL)
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
void scheduler_process(void)
{
    u32 removedEvents = __scheduler_check_timeout_and_queue();

    if (kGlobal.scheduler.needDefrag || removedEvents > 0)
        __scheduler_defrag();

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
 * 
 * @retval ScheduleEvent use it to unschedule the event, if needed
 */
ScheduleEvent schedule(u32 timeoutTicks, void (*callback)(void* arg0), void* arg0, ibool isRoutine)
{
    if (kGlobal.scheduler.eventBufferHead >= SCHEDULER_BUFF_SIZE)
        kernel_panic(KERR_SCHEDULER_OVERFLOW, "Too many scheduled events/routines.");
    
    ExTick timeoutExTick = kGlobal.timing.tick;
    extick_add_ticks(&timeoutExTick, timeoutTicks);

    ScheduleEvent ev = (ScheduleEvent){
        .timeoutTick = timeoutExTick,
        .intervalTick = timeoutTicks,
        .arg0 = arg0,
        .callback = callback,
        .isRoutine = isRoutine,
    };

    kGlobal.scheduler.eventBuffer[kGlobal.scheduler.eventBufferHead] = ev;
    ++kGlobal.scheduler.eventBufferHead;
    return ev;
}

/**
 * @brief Removes every events or routines with the provided signature.
 * 
 * For example, if 3 events are scheduled with the exact same `timeoutTicks`,
 * `*callback`, `arg0` and `isRoutine` then all those events will be unscheduled.
 * 
 * Example:
 * ```c
 * schedule(10, my_callback, NULL, false);
 * // This will remove the event
 * unschedule(10, my_callback, NULL, false);
 * // This won't
 * unschedule(11, my_callback, NULL, false);
 * ```
 * 
 * @param timeoutTicks
 * @param callback
 * @param arg0
 * @param isRoutine
 */
void unschedule(ScheduleEvent ev)
{
    for (u32 i = 0; i < kGlobal.scheduler.eventBufferHead; ++i)
    {
        ScheduleEvent *iev = &kGlobal.scheduler.eventBuffer[i];

        if (iev->callback == ev.callback
            && iev->arg0 == ev.arg0
            && iev->intervalTick == ev.intervalTick
            && iev->isRoutine == ev.isRoutine)
        {
            kGlobal.scheduler.eventBuffer[i].timeoutTick.cycle = U32_MAXVAL;
            kGlobal.scheduler.eventBuffer[i].timeoutTick.tick = 0;
            kGlobal.scheduler.eventBuffer[i].isRoutine = false;
            kGlobal.scheduler.needDefrag = true;
        }
    }
}


