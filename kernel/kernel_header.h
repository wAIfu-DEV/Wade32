#pragma once

#include "kernel_globals.h"
#include "time.h"
#include "vga_interface.h"

void kernel_draw_header(void* arg0)
{
    (void)arg0;
    VgaInterface v = vga_create_interface();
    vga_set_style(&v, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    v.updateCursor = false;

    v.xOffset = 0;
    v.yOffset = 0;

    for (u8 i = 0; i < VGA_COLS; ++i)
    {
        vga_print_char_at(' ', v.currentStyle, i, 0);
    }

    vga_print(&v, "Wade32");

    Time t = time_utc();
    HeapStr dateStr = time_to_datetime_string(&kGlobal.heap.allocator, t);
    
    v.xOffset = 7;
    v.yOffset = 0;

    vga_print(&v, "| utc:");

    if (dateStr)
    {
        vga_print(&v, dateStr);
        kGlobal.heap.allocator.free(&kGlobal.heap.allocator, dateStr);
    }
    else
    {
        vga_print(&v, "0000-00-00 00:00:00");
    }

    vga_print(&v, ", cy:");
    vga_print_uint(&v, kGlobal.timing.tick.cycle);

    vga_print(&v, ", tk:");
    vga_print_uint(&v, kGlobal.timing.tick.tick);

    vga_print(&v, ", heap:");
    vga_print_uint(&v, kGlobal.heap.debugInfo.activeBytes);
    vga_print_char(&v, '/');
    vga_print_uint(&v, KERNEL_HEAP_SIZE);
}
