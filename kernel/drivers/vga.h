#pragma once

#include "../../xstd/xstd_core.h"
#include "bios_io.h"

#define VGA_MEM (volatile i8*)0xb8000
#define VGA_MEM_BOUND (volatile i8*)0xBFFFF

#define VGA_ROWS 25
#define VGA_COLS 80

#define VGA_COLOR_BLACK         0x0
#define VGA_COLOR_BLUE          0x1
#define VGA_COLOR_GREEN         0x2
#define VGA_COLOR_CYAN          0x3
#define VGA_COLOR_RED           0x4
#define VGA_COLOR_MAGENTA       0x5
#define VGA_COLOR_BROWN         0x6
#define VGA_COLOR_LIGHT_GREY    0x7
#define VGA_COLOR_DARK_GREY     0x8
#define VGA_COLOR_LIGHT_BLUE    0x9
#define VGA_COLOR_LIGHT_GREEN   0xA
#define VGA_COLOR_LIGHT_CYAN    0xB
#define VGA_COLOR_LIGHT_RED     0xC
#define VGA_COLOR_LIGHT_MAGENTA 0xD
#define VGA_COLOR_LIGHT_BROWN   0xE
#define VGA_COLOR_WHITE         0xF

u16 vga_driver_make_entry(const i8 c, const i8 style)
{
    return (u16)c | ((u16)style << 8);
}

i8 vga_driver_make_style(const i8 foreground, const i8 background)
{
    return foreground | (background << 4);
}

void vga_driver_set_cursor(const u8 x, const u8 y)
{
    const u16 position = (u16)(y * VGA_COLS + x);
    bios_outb(0x3D4, 0x0F);
    bios_outb(0x3D5, (u8)(position & 0xFF));
    bios_outb(0x3D4, 0x0E);
    bios_outb(0x3D5, (u8)((position >> 8) & 0xFF));
}

void vga_driver_put_entry_offset(const u16 entry, const u32 off)
{
    volatile u16* offset = ((volatile u16*)VGA_MEM) + off;
    if ((u32)offset >= (u32)VGA_MEM_BOUND) return;
    *offset = entry;
}

void vga_driver_put_entry(const u16 entry, const u8 x, const u8 y)
{
    if (x >= VGA_COLS || y >= VGA_ROWS) return;
    vga_driver_put_entry_offset(entry, ((u32)y * 80) + (u32)x);
}

void vga_driver_put_char(const i8 c, const i8 style, const u8 x, const u8 y)
{
    if (x >= VGA_COLS || y >= VGA_ROWS) return;
    const u16 entry = vga_driver_make_entry(c, style);
    vga_driver_put_entry(entry, x, y);
}

// Returns 0 on out of bounds read.
u16 vga_driver_read_entry_offset(const u32 off)
{
    volatile u16* offset = ((volatile u16*)VGA_MEM) + off;
    if ((u32)offset >= (u32)VGA_MEM_BOUND)
        return 0;
    return *offset;
}

u16 vga_driver_read_entry(const u8 x, const u8 y)
{
    return vga_driver_read_entry_offset(((u32)y * 80) + (u32)x);
}
