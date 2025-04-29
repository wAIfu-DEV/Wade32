#pragma once

#include "../shared/kapp.h"
#include "../../drivers/shutdown.h"

KappReturn kapp_shutdown(void)
{
    shutdown();

    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = NULL,
    };
}
