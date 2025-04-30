#pragma once

#include "../shared/kapp.h"
#include "../../time.h"
#include "../../types/args.h"

KappReturn kapp_utc(Args args)
{
    Error err = ERR_OK;

    ResultKSTDO stdoRes = kapp_request_stdout();
    if (stdoRes.error)
        return (KappReturn){
            .errcode = stdoRes.error,
            .outOrNull = NULL,
        };
    
    KappStdout kappStdout = stdoRes.value;
    Writer* stdout = (Writer*)&kappStdout.gsw;

    struct flags
    {
        ibool displayDate;
        ibool displayTime;
    } utcFlags = {
        .displayDate = false,
        .displayTime = false,
    };

    if (args.size > 1)
    {
        for (u32 i = 1; i < args.size; ++i)
        {
            ConstStr arg = args.strings[i];

            if (!string_starts_with(arg, "-"))
            {
                writer_write_str(stdout, "Invalid argument, must start with '-': ");
                writer_write_str(stdout, arg);
                // Should really implement stderr
                utcFlags.displayDate = false;
                utcFlags.displayTime = false;
                err = ERR_INVALID_PARAMETER;
                goto exit_label;
            }

            if (string_equals(arg, "-h") || string_equals(arg, "-help"))
            {
                writer_write_str(stdout, "-h -help: Display available arguments\n");
                writer_write_str(stdout, "-t -time: Display only UTC time\n");
                writer_write_str(stdout, "-d -date: Display only UTC date\n");
            }
            else if (string_equals(arg, "-t") || string_equals(arg, "-time"))
            {
                utcFlags.displayTime = true;
            }
            else if (string_equals(arg, "-d") || string_equals(arg, "-date"))
            {
                utcFlags.displayDate = true;
            }
            else
            {
                writer_write_str(stdout, "Invalid argument: ");
                writer_write_str(stdout, arg);

                utcFlags.displayDate = false;
                utcFlags.displayTime = false;
                err = ERR_INVALID_PARAMETER;
                goto exit_label;
            }
        }
    }

    Time t = time_utc();

    if (!utcFlags.displayDate && !utcFlags.displayTime)
    {
        HeapStr datetimeStr = time_to_datetime_string(&kGlobal.heap.allocator, t);
        if (!datetimeStr)
            return (KappReturn){
                .errcode = ERR_OUT_OF_MEMORY,
                .outOrNull = NULL,
            };

        writer_write_str(stdout, datetimeStr);
        kGlobal.heap.allocator.free(&kGlobal.heap.allocator, datetimeStr);
    }
    
    if (utcFlags.displayDate) 
    {
        HeapStr dateStr = time_to_date_string(&kGlobal.heap.allocator, t);
        if (!dateStr)
            return (KappReturn){
                .errcode = ERR_OUT_OF_MEMORY,
                .outOrNull = NULL,
            };

        writer_write_str(stdout, dateStr);
        kGlobal.heap.allocator.free(&kGlobal.heap.allocator, dateStr);
    }

    if (utcFlags.displayTime)
    {
        if (utcFlags.displayDate)
        {
            writer_write_str(stdout, " ");
        }

        HeapStr timeStr = time_to_time_string(&kGlobal.heap.allocator, t);
        if (!timeStr)
            return (KappReturn){
                .errcode = ERR_OUT_OF_MEMORY,
                .outOrNull = NULL,
            };

        writer_write_str(stdout, timeStr);
        kGlobal.heap.allocator.free(&kGlobal.heap.allocator, timeStr);
    }

exit_label:
    return (KappReturn){
        .errcode = err,
        .outOrNull = kappStdout.gsw.str,
    };
}
