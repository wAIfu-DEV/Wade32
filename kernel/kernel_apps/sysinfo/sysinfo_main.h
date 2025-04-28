#pragma once

#include "../../../xstd/xstd_error.h"
#include "../../../xstd/xstd_writer.h"

#include "../../kapp_stdout.h"

#define WADE_ASCII_0 " __   _   __   _____   ______   _____\n"
#define WADE_ASCII_1 "|  | / | /  | /     | |   _  | |  __/\n"
#define WADE_ASCII_2 "|  |/  |/  /  |  /| | |  | | | |  _| \n"
#define WADE_ASCII_3 "|         /   |  || | |  |/ /  | |___\n"
#define WADE_ASCII_4 "|___/|___/    |__||_| |____/   |____/\n\n"

KappReturn kapp_sysinfo(void)
{
    ResultKSTDO stdoRes = kapp_request_stdout();
    if (stdoRes.error)
        return (KappReturn){
            .errcode = stdoRes.error,
            .outOrNull = NULL,
        };
    
    KappStdout kappStdout = stdoRes.value;
    Writer* stdout = (Writer*)&kappStdout.gsw;

    writer_write_str(stdout, WADE_ASCII_0);
    writer_write_str(stdout, WADE_ASCII_1);
    writer_write_str(stdout, WADE_ASCII_2);
    writer_write_str(stdout, WADE_ASCII_3);
    writer_write_str(stdout, WADE_ASCII_4);

    writer_write_str(stdout, "Arch: x86 - 32bit");

    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = kappStdout.gsw.str,
    };;
}
