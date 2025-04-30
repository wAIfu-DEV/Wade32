#pragma once

#include "../shared/kapp.h"
#include "../../types/args.h"


void __kapp_help_print_apps(Buffer key, void *value, void *userArg)
{
    (void)value;
    Writer* stdout = (Writer*)userArg;

    ConstBuff buff = (ConstBuff){ .bytes = key.bytes, .size = key.size };
    writer_write_bytes(stdout, buff);
    writer_write_byte(stdout, '\n');
}

KappReturn kapp_help(Args args)
{
    (void)args;

    ResultKSTDO stdoRes = kapp_request_stdout();
    if (stdoRes.error)
        return (KappReturn){
            .errcode = stdoRes.error,
            .outOrNull = NULL,
        };
    
    KappStdout kappStdout = stdoRes.value;
    Writer* stdout = (Writer*)&kappStdout.gsw;

    writer_write_str(stdout, "Available commands:\n");
    hashmap_for_each(&kGlobal.kernel_apps.registry, __kapp_help_print_apps, (void*)stdout);
    writer_write_str(stdout, "quit");

    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = kappStdout.gsw.str,
    };
}
