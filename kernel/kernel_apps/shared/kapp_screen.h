#pragma once

#include "../../../xstd/xstd_core.h"
#include "../../../xstd/xstd_error.h"
#include "../../../xstd/xstd_vectors.h"
#include "../../../xstd/xstd_rects.h"

#include "../../kernel_globals.h"
#include "../../vga_interface.h"

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

KappScreenBuffer kapp_create_screen_buff(const Buffer buff, const Rectu8 subScreenRect)
{
    return (KappScreenBuffer){
        .vgaBuffer = buff,
        .screenRect = subScreenRect,
        .cursor = (Vec2u8){0,0},
        .vgaStyle = 0x07,
    };
}

ResultKASB kapp_request_screen_buffer(Rectu8 dimensions, const ibool isFullscreen)
{
    u8 maxWidth = VGA_COLS;
    u8 maxHeight = isFullscreen ? VGA_ROWS : VGA_ROWS - 1;

    if (dimensions.x + dimensions.width > maxWidth
        || dimensions.y + dimensions.height > maxHeight)
        return (ResultKASB){
            .error = ERR_INVALID_PARAMETER,  
        };
    
    if (!isFullscreen)
    {
        // Shift down for kernel header
        ++dimensions.y;
    }

    u32 allocSize = (u32)dimensions.width * (u32)dimensions.height * 2;
    i8* bytes = kGlobal.heap.allocator.alloc(&kGlobal.heap.allocator, allocSize);

    if (!bytes)
        return (ResultKASB){
            .error = ERR_OUT_OF_MEMORY,
        };
    
    HeapBuff buff = (HeapBuff){
        .bytes = bytes,
        .size = allocSize,
    };
    
    return (ResultKASB){
        .value = kapp_create_screen_buff(buff, dimensions),
        .error = ERR_OK,
    };
}

void kapp_screen_buffer_deinit(KappScreenBuffer* sb)
{
    kGlobal.heap.allocator.free(&kGlobal.heap.allocator, sb->vgaBuffer.bytes);
}

void kapp_flush_screen_buffer(KappScreenBuffer* sb)
{
    for (u8 y = 0; y < sb->screenRect.height; ++y)
    {
        for (u8 x = 0; x < sb->screenRect.width; ++x)
        {
            u16* buffMem = ((u16*)sb->vgaBuffer.bytes) + ((y * sb->screenRect.width) + (x));
            vga_print_entry_at(*buffMem, x + sb->screenRect.x, y + sb->screenRect.y);
        }
    }
    vga_set_cursor_position(&kGlobal.screen.vga, sb->cursor.x + sb->screenRect.x, sb->cursor.y + sb->screenRect.y);
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

