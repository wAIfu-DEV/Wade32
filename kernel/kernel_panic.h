#pragma once

#include "vga.h"

void kernel_hang(void)
{
    while (true)
    {
        __asm__("hlt");
    }
}

void kernel_panic(u32 errorCode, ConstStr message)
{
    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear_screen(&v);
    vga_print(&v, "Kernel Panic.\n");
    vga_print(&v, "Error Code: ");
    vga_print_hex(&v, errorCode);
    vga_print(&v, "\n");
    vga_print(&v, message);
    vga_print_char(&v, '\n');
    kernel_hang();
}
