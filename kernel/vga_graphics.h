#pragma once

#include "../xstd/xstd_vectors.h"

#include "vga.h"

#define VGA_GFX_BORDER_VERT (u8)186
#define VGA_GFX_BORDER_HORI (u8)205
#define VGA_GFX_BORDER_TOPL (u8)201
#define VGA_GFX_BORDER_TOPR (u8)187
#define VGA_GFX_BORDER_BOTL (u8)200
#define VGA_GFX_BORDER_BOTR (u8)188

#define VGA_GFX_LINE_VERT (u8)179
#define VGA_GFX_LINE_HORI (u8)196
#define VGA_GFX_LINE_TOPL (u8)218
#define VGA_GFX_LINE_TOPR (u8)191
#define VGA_GFX_LINE_BOTL (u8)192
#define VGA_GFX_LINE_BOTR (u8)217

void vga_gfx_draw_line(VgaInterface* v, Vec2u8 topLeft, Vec2u8 botRight, ibool isHorizontal)
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

void vga_gfx_draw_border_line(VgaInterface* v, Vec2u8 topLeft, Vec2u8 botRight, ibool isHorizontal)
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

void vga_gfx_draw_rect(VgaInterface* v, Vec2u8 topLeft, Vec2u8 botRight)
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


void vga_gfx_draw_border_rect(VgaInterface* v, Vec2u8 topLeft, Vec2u8 botRight)
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
