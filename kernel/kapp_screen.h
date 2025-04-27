#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_error.h"
#include "../xstd/xstd_vectors.h"
#include "../xstd/xstd_rects.h"

typedef struct
{
    const HeapBuff vgaBuffer;
    const Rectu8 screenRect;
    Vec2u8 cursor;
    u8 vgaStyle;
} KappScreenBuffer;

typedef struct
{
    KappScreenBuffer value;
    Error error;
} ResultKASB;



void kapp_screen_set_vga_style(KappScreenBuffer*sb, u8 foreground, u8 background)
{
    sb->vgaStyle = foreground | (background << 4);
}

u16 __kapp_screen_vga_entry(char c, u8 color)
{
    return (u16)c | ((u16)color << 8);
}

KappScreenBuffer create_kapp_screen_buff(const Buffer buff, Rectu8 subScreenRect)
{
    return (KappScreenBuffer){
        .vgaBuffer = buff,
        .screenRect = subScreenRect,
        .cursor = (Vec2u8){0,0},
        .vgaStyle = 0x07,
    };
}

void kapp_screen_set_cursor_position(KappScreenBuffer* sb, u32 x, u32 y) {
    sb->cursor.x = x;
    sb->cursor.y = y;
}

void __kapp_screen_newline(KappScreenBuffer* sb)
{
    sb->cursor.x = 0;
    ++sb->cursor.y;
    
    if (sb->cursor.y >= sb->screenRect.height) {
        sb->cursor.y = 0;
    }
}

void kapp_screen_write_char_at(KappScreenBuffer* sb, u8 c, u32 x, u32 y)
{
    if (x >= sb->screenRect.width || y >= sb->screenRect.height) return;

    i8* videoMem = sb->vgaBuffer.bytes + (((y * sb->screenRect.width) + (x)) * 2);
    videoMem[0] = c;
    videoMem[1] = sb->vgaStyle;
}


void kapp_screen_write_char(KappScreenBuffer* sb, i8 c)
{
    if (c == '\n')
    {
        __kapp_screen_newline(sb);
        return;
    }
    else if (c == '\r')
    {
        sb->cursor.x = 0;
        return;
    }

    kapp_screen_write_char_at(sb, c, sb->cursor.x, sb->cursor.y);
    ++sb->cursor.x;

    if (sb->cursor.x >= sb->screenRect.width)
    {
        __kapp_screen_newline(sb);
    }
}

void kapp_screen_write_str(KappScreenBuffer* sb, ConstStr text)
{
    while (*text)
    {
        kapp_screen_write_char(sb, *text);
        ++text;
    }
}

void kapp_screen_clear(KappScreenBuffer* sb) {
    u16 *vgaMem = (u16 *)sb->vgaBuffer.bytes;
    const u16 blank = __kapp_screen_vga_entry(' ', sb->vgaStyle);
    
    for (u32 i = 0; i < (u32)sb->screenRect.width * (u32)sb->screenRect.height; ++i) {
        vgaMem[i] = blank;
    }
    sb->cursor = (Vec2u8){0,0};
}

