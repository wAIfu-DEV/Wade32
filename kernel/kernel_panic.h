#pragma once

#include "vga_interface.h"
#include "cpu/halt.h"
#include "cpu/interrupts.h"
#include "random.h"
#include "kernel_errors.h"

void kernel_panic(const u32 errorCode, ConstStr message)
{
    interrupts_disable();

    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear_screen(&v);
    vga_print(&v, "Kernel Panic.\n");
    vga_print(&v, "Error Code: ");
    vga_print_hex(&v, errorCode);
    vga_print(&v, "\n");
    vga_print(&v, message);

    while (true)
    {
        halt_no_enable_ints();
    }
    
}
