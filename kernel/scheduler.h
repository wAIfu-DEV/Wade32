#pragma once

#include "../xstd/xstd_core.h"

#include "kernel_panic.h"
#include "kernel_errors.h"
#include "kernel_globals.h"

#include "cpu/timer.h"
#include "time.h"

#include "schedule_event_type.h"

void __scheduler_enqueue_timedout(const ScheduleEvent ev)
{
    if (kGlobal.scheduler.timeoutQueueHead >= SCHEDULER_BUFF_SIZE)
        kernel_panic(KERR_SCHEDULER_OVERFLOW, "Too many timed out scheduler events.");

    kGlobal.scheduler.timeoutQueue[kGlobal.scheduler.timeoutQueueHead] = ev;
    ++kGlobal.scheduler.timeoutQueueHead;
}

void __scheduler_enqueue(const ScheduleEvent ev)
{
    if (kGlobal.scheduler.scheduleQueueHead >= SCHEDULER_BUFF_SIZE)
        kernel_panic(KERR_SCHEDULER_OVERFLOW, "Too many events/routines in schedule queue.");
    
    kGlobal.scheduler.scheduleQueue[kGlobal.scheduler.scheduleQueueHead] = ev;
    ++kGlobal.scheduler.scheduleQueueHead;
}

// Checks for timed out events, calls `__scheduler_enqueue_timedout` on timeout
// returns true on events to remove, false on element to keep
ibool __scheduler_handle_timeout(const u32 i)
{
    if (kGlobal.scheduler.eventBuffer[i].timeoutTick.cycle == U32_MAXVAL)
        return true;

    if (extick_greater_than(kGlobal.scheduler.eventBuffer[i].timeoutTick, kGlobal.timing.tick))
        return false;

    if (kGlobal.scheduler.eventBuffer[i].isRoutine)
    {
        // Advance timeout tick
        ExTick prevTimeout = kGlobal.scheduler.eventBuffer[i].timeoutTick;
        extick_add_ticks(&prevTimeout, kGlobal.scheduler.eventBuffer[i].intervalTick);
        kGlobal.scheduler.eventBuffer[i].timeoutTick = prevTimeout;
        // Move to schedule queue
        __scheduler_enqueue(kGlobal.scheduler.eventBuffer[i]);
        // Forces sorting of event buffer, is there a better way of doing this??
    }

    kGlobal.scheduler.eventBuffer[i].timeoutTick.cycle = U32_MAXVAL;
    kGlobal.scheduler.eventBuffer[i].timeoutTick.tick = 0;
    __scheduler_enqueue_timedout(kGlobal.scheduler.eventBuffer[i]);
    return true;
}

void __scheduler_check_timeout_sorted(void)
{
    for (u32 i = 0; i < kGlobal.scheduler.eventBufferHead; ++i)
    {
        const ExTick evTick = kGlobal.scheduler.eventBuffer[i].timeoutTick;
        if (extick_greater_than(evTick, kGlobal.timing.tick))
        {
            // Early return, if scheduler is correctly sorted then all
            // following items will have a timeoutTick > tick
            return;
        }

        if (__scheduler_handle_timeout(i))
        {
            kGlobal.scheduler.needDefrag = true;
        }
    }
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

void __schedule_move_queued_to_buffer(void)
{
    for (u32 i = 0; i < kGlobal.scheduler.scheduleQueueHead; ++i)
    {
        if (kGlobal.scheduler.eventBufferHead >= SCHEDULER_BUFF_SIZE)
            kernel_panic(KERR_SCHEDULER_OVERFLOW, "Too many scheduled events/routines.");

        kGlobal.scheduler.eventBuffer[kGlobal.scheduler.eventBufferHead] = kGlobal.scheduler.scheduleQueue[i];
        ++kGlobal.scheduler.eventBufferHead;
    }
    kGlobal.scheduler.scheduleQueueHead = 0;
}

void __schedule_sort_ascending(void)
{
    const u32 n = kGlobal.scheduler.eventBufferHead;
    ScheduleEvent* arr = (ScheduleEvent*)kGlobal.scheduler.eventBuffer;

    for (u32 i = 1; i < n; ++i)
    {
        ScheduleEvent ev = arr[i];
        u32 j = i - 1;

        // Move elements greater than key one position ahead
        while (j != U32_MAXVAL && extick_greater_than(arr[j].timeoutTick, ev.timeoutTick))
        {
            arr[j+1] = arr[j];
            --j;
        }
        arr[j+1] = ev;
    }
}

void __scheduler_handle_queued_timedout(void)
{
    for (u32 i = 0; i < kGlobal.scheduler.timeoutQueueHead; ++i)
    {
        ScheduleEvent ev = kGlobal.scheduler.timeoutQueue[i];
        if (ev.callback != NULL)
            ev.callback(ev.arg0);
    }
    kGlobal.scheduler.timeoutQueueHead = 0;
}

/**
 * @brief Processes timed out scheduled events and defragments the scheduler on changes.
 * 
 */
void scheduler_process(void)
{
    if (kGlobal.scheduler.scheduleQueueHead > 0)
    {
        __schedule_move_queued_to_buffer();
        __schedule_sort_ascending();
    }

    __scheduler_check_timeout_sorted();

    if (kGlobal.scheduler.needDefrag)
    {
        __scheduler_defrag();
    }

    if (kGlobal.scheduler.timeoutQueueHead > 0)
    {
        __scheduler_handle_queued_timedout();
    }
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
ScheduleEvent schedule(const u32 timeoutTicks, void (*const callback)(void* arg0), void* const arg0, const ibool isRoutine)
{
    ExTick timeoutExTick = kGlobal.timing.tick;
    extick_add_ticks(&timeoutExTick, timeoutTicks);

    ScheduleEvent ev = (ScheduleEvent){
        .timeoutTick = timeoutExTick,
        .intervalTick = timeoutTicks,
        .arg0 = arg0,
        .callback = callback,
        .isRoutine = isRoutine,
    };

    // Queueing will make it easier in the future if we ever had threading
    __scheduler_enqueue(ev);
    return ev;
}

void __scheduler_invalidate_in_buffer(const ScheduleEvent ev, ScheduleEvent* buffer, const u32 size)
{
    for (u32 i = 0; i < size; ++i)
    {
        ScheduleEvent *iev = &buffer[i];

        if (iev->callback == ev.callback
            && iev->arg0 == ev.arg0
            && iev->intervalTick == ev.intervalTick
            && iev->isRoutine == ev.isRoutine)
        {
            buffer[i].timeoutTick.cycle = U32_MAXVAL;
            buffer[i].timeoutTick.tick = 0;
            buffer[i].isRoutine = false;
            kGlobal.scheduler.needDefrag = true;
        }
    }
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
void unschedule(const ScheduleEvent ev)
{
    __scheduler_invalidate_in_buffer(ev, kGlobal.scheduler.eventBuffer, kGlobal.scheduler.eventBufferHead);
    __scheduler_invalidate_in_buffer(ev, kGlobal.scheduler.scheduleQueue, kGlobal.scheduler.scheduleQueueHead);
}


