#pragma once

#include "../../xstd/xstd_error.h"
#include "../../xstd/xstd_hashmap.h"
#include "../kapp_entry_type.h"

#define KAPPS_REG_SIZE 1

typedef struct
{
    const i8* appName;
    KappEntrypoint entryPoint;
} KappRegEntry;

Error kapp_shell(void);

static const KappRegEntry kappRegistry[KAPPS_REG_SIZE] = {
    {
        .appName = "shell",
        .entryPoint = kapp_shell,
    },
};



