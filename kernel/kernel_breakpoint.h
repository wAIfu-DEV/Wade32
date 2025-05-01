#pragma once

#include "vga_interface.h"
#include "cpu/halt.h"
#include "cpu/interrupts.h"
#include "random.h"
#include "kernel_apps/shared/kapp_input.h"
#include "kernel_process_keyboard.h"

void kernel_breakpoint()
{
    KappInputBuff ib = (KappInputBuff){
        .buffHead = 0,
        .consumed = 0,
    };
    kapp_request_input_events(&ib);

    while (kapp_input_get_key(&ib) == 0)
    {
        interrupts_disable();
        keyboard_process();
        halt();
    }
    kapp_remove_input_events(&ib);
}

void kernel_breakpoint_uint(ConstStr varName, u32 val)
{
    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_set_cursor_position(&v, 0, VGA_ROWS - 1);
    vga_print(&v, varName);
    vga_print(&v, " = ");
    vga_print_uint(&v, val);
    vga_print(&v, "; Breakpoint");

    kernel_breakpoint();

    vga_print(&v, " Done.");
}

void kernel_breakpoint_int(ConstStr varName, i32 val)
{
    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_set_cursor_position(&v, 0, VGA_ROWS - 1);
    vga_print(&v, varName);
    vga_print(&v, " = ");
    vga_print_int(&v, val);
    vga_print(&v, "; Breakpoint");

    kernel_breakpoint();

    vga_print(&v, " Done.");
}

void kernel_breakpoint_str(ConstStr varName, ConstStr val)
{
    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_set_cursor_position(&v, 0, VGA_ROWS - 1);
    vga_print(&v, varName);
    vga_print(&v, " = \"");
    vga_print(&v, val);
    vga_print(&v, "\"; Breakpoint");

    kernel_breakpoint();

    vga_print(&v, " Done.");
}

