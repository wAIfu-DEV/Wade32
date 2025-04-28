#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_writer.h"
#include "kernel_globals.h"

typedef struct
{
    GrowStrWriter gsw;
} KappStdout;

typedef struct
{
    KappStdout value;
    Error error;
} ResultKSTDO;


ResultKSTDO kapp_request_stdout(void)
{
    ResultGrowStrWriter gswRes = growstrwriter_init(kGlobal.heap.allocator, 32);

    if (gswRes.error)
        return (ResultKSTDO){
            .error = gswRes.error,
        };
    
    return (ResultKSTDO){
        .value = (KappStdout){
            .gsw = gswRes.value,
        },
        .error = ERR_OK,
    };
}
