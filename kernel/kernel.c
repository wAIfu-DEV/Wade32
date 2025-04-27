#include "../xstd/xstd_core.h"
#include "../xstd/xstd_alloc_buffer.h"
#include "../xstd/xstd_alloc_debug.h"

#include "vga.h"
#include "vga_graphics.h"

#include "kernel_errors.h"
#include "kernel_panic.h"

#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/timer.h"

#include "keyboard.h"
#include "scheduler.h"
#include "kernel_globals.h"

#include "kernel_apps/shell/shell_main.h"


void __kernel_print_time(void* arg0);

__attribute__((externally_visible, used, noinline, visibility("default")))
void kernel_main(void)
{
    // Should zero out the BSS

    kGlobal.screen.vga = vga_create_interface();
    VgaInterface* vga = &kGlobal.screen.vga;

    vga_set_style(vga, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear_screen(vga);

    // Draw console header
    vga_print(vga, "Wade32\n\n");
    vga_gfx_draw_line(vga, (Vec2u8){0, 1}, (Vec2u8){80, 1}, true);

    vga_print(vga, "> Kernel loaded.\n");

    Buffer heapBuff = (Buffer){.bytes = (i8*)kGlobal.heap.heapMem, .size = KERNEL_HEAP_SIZE};
    ResultAllocator allocRes = buffer_allocator(heapBuff);

    if (allocRes.error)
    {
        kernel_panic(KERR_ALLOC_INIT_FAILURE, "Failed to create kernel heap allocator.");
    }
    Allocator heapAlloc = allocRes.value;

    ResultAllocator debugAllocRes = debug_allocator(&kGlobal.heap.debugInfo, &heapAlloc);
    if (debugAllocRes.error)
    {
        kernel_panic(KERR_ALLOC_INIT_FAILURE, "Failed to create kernel heap debug allocator.");
    }
    kGlobal.heap.allocator = debugAllocRes.value;

    vga_print(vga, "> Created kernel heap allocator.\n");

    isr_install();
    __asm__ volatile("sti");

    vga_print(vga, "> Initialized interrupts.\n");

    init_timer(50);

    vga_print(vga, "> Started tick timer.\n");

    register_interrupt_handler(IRQ1, keyboard_handler);

    vga_print(vga, "> Registered keyboard handler, switching to user input mode.\n");
    
    u32 timeout = ms_to_ticks(200, 50);
    schedule(timeout, __kernel_print_time, &vga, true);

    vga_print(vga, "> Scheduled timings printing routine.\n");

    Error shellErr = kapp_shell_main();
    if (shellErr)
        kernel_panic((u32)shellErr, ErrorToString(shellErr));

    //vga_print(&vga, "user> ");
    while (true)
    {
        // Handle keyboard input
        if (kGlobal.keyboard.inputBufferHead)
        {
            for (u32 i = 0; i < kGlobal.keyboard.inputBufferHead; ++i)
            {
                vga_print_char(vga, kGlobal.keyboard.inputBuffer[i]);
            }
            kGlobal.keyboard.inputBufferHead = 0;
        }

        // Handle scheduled events
        scheduler_process();

        __asm__ volatile("hlt");
    }

    kernel_hang();
}


void __kernel_print_time(void* arg0)
{
    (void)arg0;
    Time t = time_utc();
    HeapStr dateStr = time_to_utc_string(&kGlobal.heap.allocator, t);

    // Draw clock MS/TICKS
    VgaInterface v = kGlobal.screen.vga; // Copy
    v.moveCursor = false;

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

    vga_print(&v, ", ms:");
    vga_print_uint(&v, ticks_to_ms(kGlobal.timing.tick, 50));

    vga_print(&v, ", tk:");
    vga_print_uint(&v, kGlobal.timing.tick);

    vga_print(&v, ", heap:");
    vga_print_uint(&v, kGlobal.heap.debugInfo.activeBytes);
    vga_print_char(&v, '/');
    vga_print_uint(&v, KERNEL_HEAP_SIZE);
}
