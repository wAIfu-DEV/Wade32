#pragma once

#include "vga.h"
#include "kernel_globals.h"
#include "kapp_screen.h"
#include "kapp_input.h"
#include "kernel_process_loop.h"

ResultKASB kapp_request_screen_buffer(Rectu8 dimensions, ibool isFullscreen)
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
        .value = create_kapp_screen_buff(buff, dimensions),
        .error = ERR_OK,
    };
}

void kapp_screen_buffer_deinit(KappScreenBuffer* sb)
{
    kGlobal.heap.allocator.free(&kGlobal.heap.allocator, sb->vgaBuffer.bytes);
}

void kapp_flush_screen_buffer(KappScreenBuffer* sb)
{
    for (u32 y = 0; y < sb->screenRect.height; ++y)
    {
        for (u32 x = 0; x < sb->screenRect.width; ++x)
        {
            u16* buffMem = ((u16*)sb->vgaBuffer.bytes) + ((y * sb->screenRect.width) + (x));
            vga_print_entry_at(*buffMem, x + sb->screenRect.x, y + sb->screenRect.y);
        }
    }
    vga_set_cursor_position(&kGlobal.screen.vga, sb->cursor.x + sb->screenRect.x, sb->cursor.y + sb->screenRect.y);
}

void __kapp_sleep_callback(void *arg)
{
    volatile ibool* flag = (volatile ibool*)arg;
    *flag = true;
}

void kapp_yield()
{
    kernel_process();
}

void kapp_sleep_ticks(u32 ticks)
{
    if (!ticks)
    {
        kapp_yield();
        return;
    }

    volatile ibool done = false;
    ScheduleEvent _ = schedule(ticks, __kapp_sleep_callback, (void*)&done, false);
    (void)_;

    while (!done)
        kernel_process();
}

void kapp_sleep_ms(u32 ms)
{
    kapp_sleep_ticks(ms_to_ticks(ms));
}
