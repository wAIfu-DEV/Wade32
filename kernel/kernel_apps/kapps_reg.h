#pragma once

#include "../../xstd/xstd_error.h"
#include "../../xstd/xstd_hashmap.h"
#include "../types/kapp_entry_type.h"
#include "../types/args.h"

typedef struct
{
    const i8* appName;
    KappEntrypoint entryPoint;
} KappRegEntry;

KappReturn kapp_shell(Args);
KappReturn kapp_sysinfo(Args);
KappReturn kapp_shutdown(Args);
KappReturn kapp_help(Args);
KappReturn kapp_utc(Args);
KappReturn kapp_args(Args);

#define KAPPS_REG_SIZE 6
static const KappRegEntry kappRegistry[KAPPS_REG_SIZE] = {
    {
        .appName = "shell",
        .entryPoint = kapp_shell,
    },
    {
        .appName = "sysinfo",
        .entryPoint = kapp_sysinfo,
    },
    {
        .appName = "shutdown",
        .entryPoint = kapp_shutdown,
    },
    {
        .appName = "help",
        .entryPoint = kapp_help,
    },
    {
        .appName = "utc",
        .entryPoint = kapp_utc,
    },
    {
        .appName = "args",
        .entryPoint = kapp_args,
    },
};



