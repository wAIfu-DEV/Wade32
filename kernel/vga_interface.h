#pragma once

#include "../xstd/xstd_core.h"
#include "drivers/bios_io.h"
#include "drivers/vga.h"

typedef struct _vga_interface
{
    u8 xOffset;
    u8 yOffset;
    i8 currentStyle;
    ibool updateCursor;
} VgaInterface;

VgaInterface vga_create_interface(void)
{
    return (VgaInterface){
        .xOffset = 0,
        .yOffset = 0,
        .currentStyle = 0x07,
        .updateCursor = true,
    };
}

void vga_set_style(VgaInterface*v, const i8 foreground, const i8 background)
{
    v->currentStyle = vga_driver_make_style(foreground, background);
}

// TODO
void vga_scroll_up(const VgaInterface* v) {
    const u16 blank = vga_driver_make_entry(' ', v->currentStyle);
    
    for (u32 i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++)
    {
        u16 under = vga_driver_read_entry_offset(i + VGA_COLS);
        vga_driver_put_entry_offset(under, i);
    }
    
    for (u32 i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++) {
        vga_driver_put_entry_offset(blank, i);
    }
}

void __vga_update_cursor(const VgaInterface* v) {
    if (!v->updateCursor) return;
    vga_driver_set_cursor(v->xOffset, v->yOffset);
}

void vga_set_cursor_position(VgaInterface* v, const u8 x, const u8 y) {
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

void vga_print_char_at(const i8 c, const i8 style, const u8 x, const u8 y)
{
    vga_driver_put_char(c, style, x, y);
}

void vga_print_entry_at(const u16 entry, const u8 x, const u8 y)
{
    vga_driver_put_entry(entry, x, y);
}

void vga_print_char(VgaInterface* v, const i8 c)
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

    vga_driver_put_char(c, v->currentStyle, v->xOffset, v->yOffset);
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

i8 __vga_digit_to_char(const u32 i)
{
    if (i > 9)
        return 0;

    return "0123456789"[i];
}

u32 __vga_div_u32(u32 dividend, const u32 divisor) {
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

u32 __vga_mod_u32(const u32 a, const u32 b)
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
        vga_print_char(v, buf[j]);
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
        vga_print_char(v, buf[j]);
    }

    v->updateCursor = cursorUpdate;
    if (cursorUpdate)
        __vga_update_cursor(v);
}

void vga_print_hex(VgaInterface* v, const u32 value)
{
    ibool cursorUpdate = v->updateCursor;
    v->updateCursor = false;

    vga_print_char(v, '0');
    vga_print_char(v, 'x');

    for (i32 i = 7; i >= 0; --i) {
        u8 nibble = (value >> (i * 4)) & 0xF;
        vga_print_char(v, (i8)((nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10)));
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
    const u16 blank = vga_driver_make_entry(' ', v->currentStyle);
    for (u32 i = 0; i < 80 * 25; ++i) {
        vga_driver_put_entry_offset(blank, i);
    }
    vga_set_cursor_position(v, 0, 0);
}
