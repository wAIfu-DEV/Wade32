#pragma once

#include "../shared/kapp.h"
#include "../../kernel_exit.h"
#include "../../types/args.h"

KappReturn kapp_shutdown(Args args)
{
    (void)args;
    kernel_shutdown_on_keypress();

    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = NULL,
    };
}
