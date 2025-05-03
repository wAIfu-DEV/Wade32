#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_string.h"

#include "kernel_panic.h"

extern volatile i8 stack_canary[];

void kernel_setup_stack_canary()
{
    volatile i8* canaryStr = (volatile i8*)stack_canary;
    string_copy_unsafe("STACK CANARY", (String)canaryStr);
    //vga_print(&kGlobal.screen.vga, (ConstStr)canaryStr);
    //kernel_hang();
}

void kernel_check_stack_canary()
{
    volatile i8* canaryStr = (volatile i8*)stack_canary;

    if (canaryStr[12] != 0)
    {
        kernel_panic(KERR_STACK_CANARY_FAULT, "Stack canary fault: Overwritten canary end.");
    }

    if (!string_equals((ConstStr)canaryStr, "STACK CANARY"))
    {
        kernel_panic(KERR_STACK_CANARY_FAULT, (ConstStr)canaryStr);
    }
}
