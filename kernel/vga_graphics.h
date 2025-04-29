#pragma once

#include "../xstd/xstd_vectors.h"

#include "vga_interface.h"

#define VGA_GFX_BORDER_VERT (i8)186
#define VGA_GFX_BORDER_HORI (i8)205
#define VGA_GFX_BORDER_TOPL (i8)201
#define VGA_GFX_BORDER_TOPR (i8)187
#define VGA_GFX_BORDER_BOTL (i8)200
#define VGA_GFX_BORDER_BOTR (i8)188

#define VGA_GFX_LINE_VERT (i8)179
#define VGA_GFX_LINE_HORI (i8)196
#define VGA_GFX_LINE_TOPL (i8)218
#define VGA_GFX_LINE_TOPR (i8)191
#define VGA_GFX_LINE_BOTL (i8)192
#define VGA_GFX_LINE_BOTR (i8)217

void vga_gfx_draw_line(VgaInterface* v, const Vec2u8 topLeft, const Vec2u8 botRight, const ibool isHorizontal)
{
    if (isHorizontal)
    {
        // Draw horizontal lines
        for (u8 i = topLeft.x; i < botRight.x; ++i)
        {
            vga_print_char_at(VGA_GFX_LINE_HORI, v->currentStyle, i, topLeft.y);
            vga_print_char_at(VGA_GFX_LINE_HORI, v->currentStyle, i, botRight.y);
        }
    }
    else
    {
        // Draw vertical lines
        for (u8 i = topLeft.y; i < botRight.y; ++i)
        {
            vga_print_char_at(VGA_GFX_LINE_VERT, v->currentStyle, topLeft.x, i);
            vga_print_char_at(VGA_GFX_LINE_VERT, v->currentStyle, botRight.x, i);
        }
    }
}

void vga_gfx_draw_border_line(VgaInterface* v, const Vec2u8 topLeft, const Vec2u8 botRight, const ibool isHorizontal)
{
    if (isHorizontal)
    {
        // Draw horizontal lines
        for (u8 i = topLeft.x; i < botRight.x; ++i)
        {
            vga_print_char_at(VGA_GFX_BORDER_HORI, v->currentStyle, i, topLeft.y);
            vga_print_char_at(VGA_GFX_BORDER_HORI, v->currentStyle, i, botRight.y);
        }
    }
    else
    {
        // Draw vertical lines
        for (u8 i = topLeft.y; i < botRight.y; ++i)
        {
            vga_print_char_at(VGA_GFX_BORDER_VERT, v->currentStyle, topLeft.x, i);
            vga_print_char_at(VGA_GFX_BORDER_VERT, v->currentStyle, botRight.x, i);
        }
    }
}

void vga_gfx_draw_rect(VgaInterface* v, const Vec2u8 topLeft, const Vec2u8 botRight)
{
    // Draw corners
    vga_print_char_at(VGA_GFX_LINE_TOPL, v->currentStyle, topLeft.x, topLeft.y);
    vga_print_char_at(VGA_GFX_LINE_BOTR, v->currentStyle, botRight.x, botRight.y);
    vga_print_char_at(VGA_GFX_LINE_TOPR, v->currentStyle, botRight.x, topLeft.y);
    vga_print_char_at(VGA_GFX_LINE_BOTL, v->currentStyle, topLeft.x, botRight.y);

    // Draw horizontal lines
    for (u8 i = topLeft.x + 1; i < botRight.x; ++i)
    {
        vga_print_char_at(VGA_GFX_LINE_HORI, v->currentStyle, i, topLeft.y);
        vga_print_char_at(VGA_GFX_LINE_HORI, v->currentStyle, i, botRight.y);
    }

    // Draw vertical lines
    for (u8 i = topLeft.y + 1; i < botRight.y; ++i)
    {
        vga_print_char_at(VGA_GFX_LINE_VERT, v->currentStyle, topLeft.x, i);
        vga_print_char_at(VGA_GFX_LINE_VERT, v->currentStyle, botRight.x, i);
    }
}


void vga_gfx_draw_border_rect(VgaInterface* v, const Vec2u8 topLeft, const Vec2u8 botRight)
{
    // Draw corners
    vga_print_char_at(VGA_GFX_BORDER_TOPL, v->currentStyle, topLeft.x, topLeft.y);
    vga_print_char_at(VGA_GFX_BORDER_BOTR, v->currentStyle, botRight.x, botRight.y);
    vga_print_char_at(VGA_GFX_BORDER_TOPR, v->currentStyle, botRight.x, topLeft.y);
    vga_print_char_at(VGA_GFX_BORDER_BOTL, v->currentStyle, topLeft.x, botRight.y);

    // Draw horizontal lines
    for (u8 i = topLeft.x + 1; i < botRight.x; ++i)
    {
        vga_print_char_at(VGA_GFX_BORDER_HORI, v->currentStyle, i, topLeft.y);
        vga_print_char_at(VGA_GFX_BORDER_HORI, v->currentStyle, i, botRight.y);
    }

    // Draw vertical lines
    for (u8 i = topLeft.y + 1; i < botRight.y; ++i)
    {
        vga_print_char_at(VGA_GFX_BORDER_VERT, v->currentStyle, topLeft.x, i);
        vga_print_char_at(VGA_GFX_BORDER_VERT, v->currentStyle, botRight.x, i);
    }
}
