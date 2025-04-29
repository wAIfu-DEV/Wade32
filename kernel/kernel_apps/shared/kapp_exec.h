#pragma once

#include "../../../xstd/xstd_hashmap.h"

#include "../../kernel_globals.h"
#include "../../kernel_apps/kapps_reg.h"

typedef struct
{
    KappEntrypoint value;
    Error error;
} ResultKEP;


ResultKEP kapp_get_entrypoint(ConstStr appName)
{
    if (!appName)
        return (ResultKEP){
            .error = ERR_INVALID_PARAMETER,
        };
    
    KappEntrypoint kapp;
    Error err = hashmap_get_str(&kGlobal.kernel_apps.registry, appName, &kapp);
    if (err)
        return (ResultKEP){
            .error = err,
        };
    
    return (ResultKEP){
        .value = kapp,
        .error = ERR_OK,
    };
}
