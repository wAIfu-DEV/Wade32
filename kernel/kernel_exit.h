#pragma once

#include "../xstd/xstd_core.h"
#include "drivers/shutdown.h"
#include "vga_interface.h"
#include "kernel_hang.h"
#include "kernel_exit_nogui.h"

void kernel_shutdown_on_keypress(void)
{
    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear_screen(&v);
    vga_print(&v, "Kernel Shutdown.\nPress any key to proceed.");

    kernel_hang_until_keypress();
    kernel_shutdown();
}

void kernel_shutdown_on_3_keypress(void)
{
    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear_screen(&v);
    vga_print(&v, "Kernel Shutdown.\nPress any key 3 times to proceed.");

    kernel_hang_until_keypress();
    kernel_hang_until_keypress();
    kernel_hang_until_keypress();
    kernel_shutdown();
}
