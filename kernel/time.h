#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_alloc.h"
#include "../xstd/xstd_string.h"
#include "bios_io.h"
#include "cpu/timer.h"
#include "time_type.h"
#include "kernel_globals.h"

u32 ticks_to_ms(u32 ticks, u32 tickHz)
{
    return (ticks * 1000) / tickHz;
}

u32 ms_to_ticks(u32 ms, u32 tickHz)
{
    return (ms * tickHz) / 1000;
}

u8 read_rtc_register(u8 reg) {
    bios_outb(0x70, reg);
    return bios_inb(0x71);
}

ibool is_rtc_time_updating() {
    bios_outb(0x70, 0x0A);
    return bios_inb(0x71) & 0x80;
}

u8 bcd_to_binary(u8 bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

Time time_utc()
{
    // If last time is within reasonable range, skip waiting and return last time.
    // Unsure if needed, but should prevent blocking in most cases considering this
    // function might be used multiple times per sec on OS-scale.
    // Max accuracy loss of ~200ms
    if (kGlobal.timing.tick - kGlobal.timing.utcCacheTick < 10) {
        return kGlobal.timing.cachedUtcTime;
    } else {
        while (is_rtc_time_updating()); // Wait until not updating
    }

    kGlobal.timing.cachedUtcTime = (Time){
        .seconds = bcd_to_binary(read_rtc_register(0x00)),
        .minutes = bcd_to_binary(read_rtc_register(0x02)),
        .hours =   bcd_to_binary(read_rtc_register(0x04)),
        .day =     bcd_to_binary(read_rtc_register(0x07)),
        .month =   bcd_to_binary(read_rtc_register(0x08)),
        .year =    bcd_to_binary(read_rtc_register(0x09)),
        .century = bcd_to_binary(read_rtc_register(0x32)),
    };
    kGlobal.timing.utcCacheTick = kGlobal.timing.tick;
    return kGlobal.timing.cachedUtcTime;
}

HeapStr time_to_utc_string(Allocator* alloc, Time t)
{
    ResultStrBuilder builderRes = strbuilder_init(alloc);
    if (builderRes.error)
        return NULL;

    StringBuilder builder = builderRes.value;

    u32 realYear = (u32)t.year + ((u32)t.century * 100);
    ResultOwnedStr r = string_from_uint(alloc, realYear);
    if (r.error)
    {
        strbuilder_deinit(&builder); // Deallocates copied and owned strings
        return NULL;
    };

    strbuilder_push_owned(&builder, r.value);
    strbuilder_push_copy(&builder, "-");

    if (t.month < 10)
    {
        strbuilder_push_copy(&builder, "0");
    }

    r = string_from_uint(alloc, t.month);
    if (r.error)
    {
        strbuilder_deinit(&builder);
        return NULL;
    };

    strbuilder_push_owned(&builder, r.value);
    strbuilder_push_copy(&builder, "-");

    if (t.day < 10)
    {
        strbuilder_push_copy(&builder, "0");
    }

    r = string_from_uint(alloc, t.day);
    if (r.error)
    {
        strbuilder_deinit(&builder);
        return NULL;
    };

    strbuilder_push_owned(&builder, r.value);
    strbuilder_push_copy(&builder, " ");

    if (t.hours < 10)
    {
        strbuilder_push_copy(&builder, "0");
    }

    r = string_from_uint(alloc, t.hours);
    if (r.error)
    {
        strbuilder_deinit(&builder);
        return NULL;
    };

    strbuilder_push_owned(&builder, r.value);
    strbuilder_push_copy(&builder, ":");

    if (t.minutes < 10)
    {
        strbuilder_push_copy(&builder, "0");
    }

    r = string_from_uint(alloc, t.minutes);
    if (r.error)
    {
        strbuilder_deinit(&builder);
        return NULL;
    };

    strbuilder_push_owned(&builder, r.value);
    strbuilder_push_copy(&builder, ":");

    if (t.seconds < 10)
    {
        strbuilder_push_copy(&builder, "0");
    }

    r = string_from_uint(alloc, t.seconds);
    if (r.error)
    {
        strbuilder_deinit(&builder);
        return NULL;
    };

    strbuilder_push_owned(&builder, r.value);

    r = strbuilder_get_string(&builder);
    if (r.error)
    {
        strbuilder_deinit(&builder);
        return NULL;
    };

    strbuilder_deinit(&builder);
    return r.value;
}
