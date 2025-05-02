#pragma once

#include "kernel_hang.h"

void kernel_shutdown(void)
{
    shutdown();
}

void kernel_shutdown_on_keypress_nogui(void)
{
    kernel_hang_until_keypress();
    kernel_shutdown();
}

void kernel_shutdown_on_3_keypress_nogui(void)
{
    kernel_hang_until_keypress();
    kernel_hang_until_keypress();
    kernel_hang_until_keypress();
    kernel_shutdown();
}
