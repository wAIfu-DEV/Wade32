#pragma once

#include "../../../xstd/xstd_error.h"
#include "../../../xstd/xstd_writer.h"

#include "../shared/kapp_stdout.h"
#include "../../types/args.h"

#define WADE_ASCII_0 " __   _   __   _____   ______   _____\n"
#define WADE_ASCII_1 "|  | / | /  | /     | |   _  | |  __/\n"
#define WADE_ASCII_2 "|  |/  |/  /  |  /| | |  | | | |  _| \n"
#define WADE_ASCII_3 "|         /   |  || | |  |/ /  | |___\n"
#define WADE_ASCII_4 "|___/|___/    |__||_| |____/   |____/\n\n"

#define SYSINFO_0    "By:   w-AI-fu_DEV (devan)\n"
#define SYSINFO_1    "Arch: x86 - 32bit"

#define WADE_ASCII WADE_ASCII_0 WADE_ASCII_1 WADE_ASCII_2 WADE_ASCII_3 WADE_ASCII_4

KappReturn kapp_sysinfo(Args args)
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

    Error err;

    err = writer_write_str(stdout, WADE_ASCII);
    if (err) goto cleanup;

    err = writer_write_str(stdout, SYSINFO_0);
    if (err) goto cleanup;

    err = writer_write_str(stdout, SYSINFO_1);
    if (err) goto cleanup;

    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = kappStdout.gsw.str,
    };

cleanup:
    return (KappReturn){
    .errcode = err,
    .outOrNull = kappStdout.gsw.str,
};
}
