#pragma once

#include "../shared/kapp.h"
#include "../../drivers/shutdown.h"
#include "../../types/args.h"

KappReturn kapp_shutdown(Args args)
{
    (void)args;
    shutdown();

    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = NULL,
    };
}
