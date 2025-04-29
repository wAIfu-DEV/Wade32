#pragma once

#include "../../xstd/xstd_error.h"
#include "../../xstd/xstd_hashmap.h"
#include "../types/kapp_entry_type.h"

typedef struct
{
    const i8* appName;
    KappEntrypoint entryPoint;
} KappRegEntry;

KappReturn kapp_shell(void);
KappReturn kapp_sysinfo(void);
KappReturn kapp_shutdown(void);
KappReturn kapp_help(void);

#define KAPPS_REG_SIZE 4
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
    }
};



