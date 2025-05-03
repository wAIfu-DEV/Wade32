#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_alloc.h"
#include "../xstd/xstd_string.h"
#include "../xstd/xstd_writer.h"

#include "drivers/bios_io.h"
#include "cpu/timer.h"
#include "drivers/rtc_time.h"
#include "types/time_type.h"
#include "kernel_globals.h"

u32 ticks_to_ms(const u32 ticks)
{
    return (ticks * 1000) / kGlobal.timing.tickFreq;
}

u32 ms_to_ticks(const u32 ms)
{
    return (ms * kGlobal.timing.tickFreq) / 1000;
}

Time time_utc(void)
{
    // If last time is within reasonable range, skip waiting and return last time.
    // Unsure if needed, but should prevent blocking in most cases considering this
    // function might be used multiple times per sec on OS-scale.
    // Max accuracy loss of ~200ms
    if (kGlobal.timing.tick.tick - kGlobal.timing.utcCacheTick < 10) {
        return kGlobal.timing.cachedUtcTime;
    } else {
        while (rtc_is_updating()); // Wait until not updating
    }

    kGlobal.timing.cachedUtcTime = rtc_get_time();
    kGlobal.timing.utcCacheTick = kGlobal.timing.tick.tick;
    return kGlobal.timing.cachedUtcTime;
}

HeapStr time_to_datetime_string(Allocator* alloc, const Time t)
{
    HeapStr str = alloc->alloc(alloc, 20);

    if (!str)
        return NULL;
    
    Error err;
    
    HeapBuff buff = (HeapBuff){ .bytes = str, .size = 20 };
    ResultBuffWriter writerRes = buffwriter_init(buff);
    err = writerRes.error;
    if (err) goto cleanup;
    
    BuffWriter buffWriter = writerRes.value;
    Writer *writer = (Writer*)&buffWriter;

    const u32 realYear = (u32)t.year + ((u32)t.century * 100);
    err = writer_write_uint(writer, realYear);
    if (err) goto cleanup;

    err = writer_write_byte(writer, '-');
    if (err) goto cleanup;

    if (t.month < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.month);
    if (err) goto cleanup;

    err = writer_write_byte(writer, '-');
    if (err) goto cleanup;

    if (t.day < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.day);
    if (err) goto cleanup;

    err = writer_write_byte(writer, ' ');
    if (err) goto cleanup;

    if (t.hours < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.hours);
    if (err) goto cleanup;

    err = writer_write_byte(writer, ':');
    if (err) goto cleanup;

    if (t.minutes < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.minutes);
    if (err) goto cleanup;

    err = writer_write_byte(writer, ':');
    if (err) goto cleanup;

    if (t.seconds < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.seconds);
    if (err) goto cleanup;

    err = writer_write_byte(writer, 0);
    if (err) goto cleanup;

    return str;

cleanup:
    alloc->free(alloc, str);
    return NULL;
}

HeapStr time_to_date_string(Allocator* alloc, const Time t)
{
    HeapStr str = alloc->alloc(alloc, 20);

    if (!str)
        return NULL;
    
    Error err;
    
    HeapBuff buff = (HeapBuff){ .bytes = str, .size = 11 };
    ResultBuffWriter writerRes = buffwriter_init(buff);
    err = writerRes.error;
    if (err) goto cleanup;
    
    BuffWriter buffWriter = writerRes.value;
    Writer *writer = (Writer*)&buffWriter;

    const u32 realYear = (u32)t.year + ((u32)t.century * 100);
    err = writer_write_uint(writer, realYear);
    if (err) goto cleanup;

    err = writer_write_byte(writer, '-');
    if (err) goto cleanup;

    if (t.month < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.month);
    if (err) goto cleanup;

    err = writer_write_byte(writer, '-');
    if (err) goto cleanup;

    if (t.day < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.day);
    if (err) goto cleanup;

    err = writer_write_byte(writer, 0);
    if (err) goto cleanup;

    return str;

cleanup:
    alloc->free(alloc, str);
    return NULL;
}

HeapStr time_to_time_string(Allocator* alloc, const Time t)
{
    HeapStr str = alloc->alloc(alloc, 20);

    if (!str)
        return NULL;
    
    Error err;
    
    HeapBuff buff = (HeapBuff){ .bytes = str, .size = 9 };
    ResultBuffWriter writerRes = buffwriter_init(buff);
    err = writerRes.error;
    if (err) goto cleanup;
    
    BuffWriter buffWriter = writerRes.value;
    Writer *writer = (Writer*)&buffWriter;

    if (t.hours < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.hours);
    if (err) goto cleanup;

    err = writer_write_byte(writer, ':');
    if (err) goto cleanup;

    if (t.minutes < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.minutes);
    if (err) goto cleanup;

    err = writer_write_byte(writer, ':');
    if (err) goto cleanup;

    if (t.seconds < 10)
    {
        err = writer_write_byte(writer, '0');
        if (err) goto cleanup;
    }

    err = writer_write_uint(writer, t.seconds);
    if (err) goto cleanup;

    err = writer_write_byte(writer, 0);
    if (err) goto cleanup;

    return str;

cleanup:
    alloc->free(alloc, str);
    return NULL;
}
