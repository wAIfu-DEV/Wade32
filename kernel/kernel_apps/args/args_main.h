#pragma once

#include "../shared/kapp.h"
#include "../../types/args.h"

KappReturn kapp_args(Args args)
{
    if (args.size < 2)
        return (KappReturn){
            .errcode = ERR_INVALID_PARAMETER,
        };

    ResultKSTDO stdoRes = kapp_request_stdout();
    if (stdoRes.error)
        return (KappReturn){
            .errcode = stdoRes.error,
            .outOrNull = NULL,
        };
    
    KappStdout kappStdout = stdoRes.value;
    Writer* stdout = (Writer*)&kappStdout.gsw;

    for (u32 i = 1; i < args.size; ++i)
    {
        writer_write_str(stdout, args.strings[i]);

        if (i != args.size - 1)
            writer_write_byte(stdout, '\n');
    }

    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = kappStdout.gsw.str,
    };
}
