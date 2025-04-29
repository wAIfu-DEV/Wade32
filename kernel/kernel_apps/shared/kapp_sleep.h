#pragma once

#include "../../time.h"
#include "../../scheduler.h"

void __kapp_sleep_callback(void *arg)
{
    volatile ibool* flag = (volatile ibool*)arg;
    *flag = true;
}

void kapp_yield(void)
{
    kernel_process();
}

void kapp_sleep_ticks(const u32 ticks)
{
    if (!ticks)
    {
        kapp_yield();
        return;
    }

    volatile ibool done = false;
    ScheduleEvent _ = schedule(ticks, __kapp_sleep_callback, (void*)&done, false);
    (void)_;

    while (!done)
        kernel_process();
}

void kapp_sleep_ms(const u32 ms)
{
    kapp_sleep_ticks(ms_to_ticks(ms));
}
