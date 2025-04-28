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
    i8 vgaStyle;
} KappScreenBuffer;

typedef struct
{
    KappScreenBuffer value;
    Error error;
} ResultKASB;



void kapp_screen_set_vga_style(KappScreenBuffer* sb, const i8 foreground, const i8 background)
{
    sb->vgaStyle = foreground | (background << 4);
}

u16 __kapp_screen_vga_entry(const i8 c, const i8 color)
{
    return (u16)c | ((u16)color << 8);
}

void kapp_screen_scroll_up(const KappScreenBuffer* sb)
{
    u16 *vgaMem = (u16 *)sb->vgaBuffer.bytes;
    const u16 blank = __kapp_screen_vga_entry(' ', sb->vgaStyle);

    const u32 rows = sb->screenRect.height;
    const u32 cols = sb->screenRect.width;
    
    for (u32 i = 0; i < (rows - 1) * cols; i++) {
        vgaMem[i] = vgaMem[i + cols];
    }
    
    for (u32 i = (rows - 1) * cols; i < rows * cols; i++) {
        vgaMem[i] = blank;
    }
}

KappScreenBuffer create_kapp_screen_buff(const Buffer buff, const Rectu8 subScreenRect)
{
    return (KappScreenBuffer){
        .vgaBuffer = buff,
        .screenRect = subScreenRect,
        .cursor = (Vec2u8){0,0},
        .vgaStyle = 0x07,
    };
}

void kapp_screen_set_cursor_position(KappScreenBuffer* sb, const u8 x, const u8 y) {
    sb->cursor.x = x;
    sb->cursor.y = y;
}

void __kapp_screen_newline(KappScreenBuffer* sb)
{
    sb->cursor.x = 0;
    ++sb->cursor.y;
    
    if (sb->cursor.y >= sb->screenRect.height) {
        kapp_screen_scroll_up(sb);
        --sb->cursor.y;
    }
}

void kapp_screen_write_char_at(const KappScreenBuffer* sb, const i8 c, const u8 x, const u8 y)
{
    if (x >= sb->screenRect.width || y >= sb->screenRect.height) return;

    i8* videoMem = sb->vgaBuffer.bytes + (((y * sb->screenRect.width) + (x)) * 2);
    videoMem[0] = c;
    videoMem[1] = sb->vgaStyle;
}

void __kapp_screen_advance_cursor(KappScreenBuffer* sb)
{
    ++sb->cursor.x;

    if (sb->cursor.x >= sb->screenRect.width && sb->cursor.y < sb->screenRect.height)
    {
        sb->cursor.x = 0;
        ++sb->cursor.y;
    }
}

void __kapp_screen_retreat_cursor(KappScreenBuffer* sb)
{
    if (sb->cursor.x == 0 && sb->cursor.y > 0)
    {
        --sb->cursor.y;
        sb->cursor.x = sb->screenRect.width;
    }

    --sb->cursor.x;
}

void kapp_screen_write_char(KappScreenBuffer* sb, const i8 c)
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

