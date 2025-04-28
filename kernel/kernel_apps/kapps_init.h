#pragma once

#include "../../xstd/xstd_error.h"
#include "../../xstd/xstd_hashmap.h"

#include "../kapp_entry_type.h"
#include "../kernel_globals.h"
#include "kapps.h"

Error kernel_kappreg_init(void)
{
    ResultHashMap envMapRes = HashMapInitT(KappEntrypoint, &kGlobal.heap.allocator);
    if (envMapRes.error)
        return envMapRes.error;

    kGlobal.kernel_apps.registry = envMapRes.value;

    for (u32 i = 0; i < KAPPS_REG_SIZE; ++i)
    {
        const KappRegEntry entry = kappRegistry[i];
        Error err = hashmap_set_str(&kGlobal.kernel_apps.registry, entry.appName, &entry.entryPoint);
        if (err)
            return err;
    }

    return ERR_OK;
}
