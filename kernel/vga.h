#pragma once

#include "../xstd/xstd_core.h"
#include "bios_io.h"

#define VGA_MEM (volatile i8*)0xb8000

#define VGA_ROWS 25
#define VGA_COLS 80
#define VGA_MAX_OFFSET 2000

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

typedef struct _vga_interface
{
    u32 xOffset;
    u32 yOffset;
    u8 currentStyle;
    ibool updateCursor;
} VgaInterface;

VgaInterface vga_create_interface()
{
    return (VgaInterface){
        .xOffset = 0,
        .yOffset = 0,
        .currentStyle = 0x07,
        .updateCursor = true,
    };
}

void vga_set_style(VgaInterface*v, u8 foreground, u8 background)
{
    v->currentStyle = foreground | (background << 4);
}

u16 __vga_entry(char c, u8 color)
{
    return (u16)c | ((u16)color << 8);
}

void vga_scroll_up(VgaInterface* v) {
    volatile u16 *vgaMem = (u16 *)VGA_MEM;
    const u16 blank = __vga_entry(' ', v->currentStyle);
    
    for (u32 i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++) {
        vgaMem[i] = vgaMem[i + VGA_COLS];
    }
    
    for (u32 i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++) {
        vgaMem[i] = blank;
    }
}

void __vga_update_cursor(VgaInterface* v) {
    if (!v->updateCursor) return;

    u16 position = v->yOffset * VGA_COLS + v->xOffset;
    
    bios_outb(0x3D4, 0x0F);
    bios_outb(0x3D5, (u8)(position & 0xFF));
    bios_outb(0x3D4, 0x0E);
    bios_outb(0x3D5, (u8)((position >> 8) & 0xFF));
}

void vga_set_cursor_position(VgaInterface* v, u32 x, u32 y) {
    v->xOffset = x;
    v->yOffset = y;
    __vga_update_cursor(v);
}

void __vga_newline(VgaInterface* v)
{
    v->xOffset = 0;
    ++v->yOffset;
    
    if (v->yOffset >= VGA_ROWS) {
        vga_scroll_up(v);
        v->yOffset = VGA_ROWS - 1;
    }
    __vga_update_cursor(v);
}

void vga_print_char_at(u8 c, u8 style, u32 x, u32 y)
{
    if (x >= VGA_COLS || y >= VGA_ROWS) return;

    volatile i8* videoMem = VGA_MEM + (((y * 80) + (x)) * 2);
    videoMem[0] = c;
    videoMem[1] = style;
}

void vga_print_entry_at(u16 entry, u32 x, u32 y)
{
    if (x >= VGA_COLS || y >= VGA_ROWS) return;

    volatile u16* videoMem = ((volatile u16*)VGA_MEM) + ((y * 80) + (x));
    *videoMem = entry;
}

void vga_print_char(VgaInterface* v, u8 c)
{
    if (c == '\n') 
    {
        __vga_newline(v);
        return;
    }
    else if (c == '\r')
    {
        v->xOffset = 0;
        __vga_update_cursor(v);
        return;
    }

    vga_print_char_at(c, v->currentStyle, v->xOffset, v->yOffset);
    ++v->xOffset;

    if (v->xOffset >= VGA_COLS)
    {
        __vga_newline(v);
    }
    else
    {
        __vga_update_cursor(v);
    }
}

i8 __vga_digit_to_char(u32 i)
{
    if (i > 9)
        return 0;

    return "0123456789"[i];
}

u32 __vga_div_u32(u32 dividend, u32 divisor) {
    if (divisor == 0)
        return 0; // You might want to kernel panic or assert here

    u32 quotient = 0;

    for (i32 i = 31; i >= 0; --i) {
        if (divisor <= (dividend >> i)) {
            dividend -= (divisor << i);
            quotient |= ((u32)1 << i);
        }
    }

    return quotient;
}

u32 __vga_mod_u32(u32 a, u32 b)
{
    return a - (b * __vga_div_u32(a, b));
}

void vga_print_uint(VgaInterface* v, const u32 i)
{
    ibool cursorUpdate = v->updateCursor;
    v->updateCursor = false;

    i8 buf[20];
    i16 idx = 0;
    u32 n = i;

    if (n == 0)
    {
        vga_print_char(v, (u8)'0');
        return;
    }

    while (n != 0)
    {
        u32 d = __vga_mod_u32(n, 10);
        buf[idx++] = __vga_digit_to_char(d);
        n = __vga_div_u32(n, 10);
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        vga_print_char(v, (u8)buf[j]);
    }

    v->updateCursor = cursorUpdate;
    if (cursorUpdate)
        __vga_update_cursor(v);
}

void vga_print_int(VgaInterface* v, const i32 i)
{
    ibool cursorUpdate = v->updateCursor;
    v->updateCursor = false;

    i8 buf[20];
    i32 n = i;
    i16 idx = 0;

    if (n == 0)
    {
        vga_print_char(v, (u8)'0');
        return;
    }

    if (n < 0)
    {
        vga_print_char(v, (u8)'-');
        n = -n;
    }

    u32 un = (u32)n;

    while (un != 0)
    {
        u32 d = __vga_mod_u32(un, 10);
        buf[idx++] = __vga_digit_to_char(d);
        un = __vga_div_u32(un, 10);
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        vga_print_char(v, (u8)buf[j]);
    }

    v->updateCursor = cursorUpdate;
    if (cursorUpdate)
        __vga_update_cursor(v);
}

void vga_print_hex(VgaInterface* v, u32 value)
{
    ibool cursorUpdate = v->updateCursor;
    v->updateCursor = false;

    vga_print_char(v, '0');
    vga_print_char(v, 'x');

    for (i32 i = 7; i >= 0; --i) {
        u8 nibble = (value >> (i * 4)) & 0xF;
        vga_print_char(v, (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10));
    }

    v->updateCursor = cursorUpdate;
    if (cursorUpdate)
        __vga_update_cursor(v);
}

void vga_print(VgaInterface* v, ConstStr text)
{
    ibool cursorUpdate = v->updateCursor;
    v->updateCursor = false;

    while (*text)
    {
        vga_print_char(v, *text);
        ++text;
    }

    v->updateCursor = cursorUpdate;
    if (cursorUpdate)
        __vga_update_cursor(v);
}

void vga_clear_screen(VgaInterface* v) {
    volatile u16 *vgaMem = (u16 *)VGA_MEM;
    const u16 blank = __vga_entry(' ', v->currentStyle);
    
    for (u32 i = 0; i < 80 * 25; ++i) {
        vgaMem[i] = blank;
    }
    vga_set_cursor_position(v, 0, 0);
}
