#pragma once

#include "../xstd/xstd_hashmap.h"

#include "kernel_globals.h"
#include "kernel_apps/kapps_reg.h"

Error kapp_exec(ConstStr appName)
{
    KappEntrypoint kapp;
    Error err = hashmap_get_str(&kGlobal.kernel_apps.registry, appName, &kapp);
    if (err)
        return err;

    return kapp();
}
