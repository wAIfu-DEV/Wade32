#include "../xstd/xstd_core.h"
#include "../xstd/xstd_alloc_buffer.h"
#include "../xstd/xstd_alloc_debug.h"

#include "vga_interface.h"
#include "vga_graphics.h"

#include "kernel_errors.h"
#include "kernel_panic.h"

#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/timer.h"
#include "cpu/interrupts.h"

#include "drivers/keyboard.h"
#include "scheduler.h"
#include "kernel_globals.h"
#include "kernel_process_loop.h"

#include "kernel_apps/kapps_reg.h"
#include "kernel_apps/kapps_init.h"
#include "kernel_apps/shared/kapp_exec.h"

#include "random.h"

void __kernel_print_time(void* arg0);

/**
 * @brief Entry point of the kernel.
 * 
 */
__attribute__((externally_visible, used, noinline, visibility("default")))
void kernel_main(void)
{
    // We should zero out the BSS
    Error err;

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
    interrupts_enable();

    vga_print(vga, "> Initialized interrupts.\n");

    timer_init(KERNEL_TICK_FREQ);

    vga_print(vga, "> Started tick timer.\n");

    register_interrupt_handler(IRQ1, keyboard_driver_in_handler);

    vga_print(vga, "> Registered keyboard handler, switching to user input mode.\n");
    
    u32 timeout = ms_to_ticks(KERNEL_PRINT_TIME_INTERVAL_MS);
    schedule(timeout, __kernel_print_time, &vga, true);

    vga_print(vga, "> Scheduled timings printing routine.\n");

    err = kernel_kappreg_init();
    if (err)
        kernel_panic(KERR_ENV_INIT_ERROR, "Failed to initialize kernel apps registry.");
    
    vga_print(vga, "> Initialized kernel apps registry.\n");

    vga_clear_screen(vga);
    vga_print(vga, "Wade32\n\n");

    Time t = time_utc();
    pseudorandom_set_seed((u32)t.seconds + (u32)t.minutes + (u32)t.hours);

    ResultKEP kepRes = kapp_get_entrypoint("shell");
    if (kepRes.error)
        kernel_panic(KERR_NO_SHELL, "Failed to find kernel shell.");
    
    Args shellArgs = (Args){
        .size = 0,
        .strings = NULL,
    };

    KappEntrypoint shell = kepRes.value;
    KappReturn ret = shell(shellArgs);
    if (ret.errcode)
        kernel_panic((u32)ret.errcode, ErrorToString(ret.errcode));

    while (true)
        kernel_process();

    kernel_hang();
}

void __kernel_print_time(void* arg0)
{
    (void)arg0;
    Time t = time_utc();
    HeapStr dateStr = time_to_datetime_string(&kGlobal.heap.allocator, t);

    VgaInterface v = kGlobal.screen.vga; // Copy
    v.updateCursor = false;

    v.xOffset = 7;
    v.yOffset = 0;

    u32 i = 0;
    while (i < VGA_COLS - 7)
    {
        vga_print_char(&v, ' ');
        ++i;
    }
    
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

    vga_print(&v, ", cy:");
    vga_print_uint(&v, kGlobal.timing.tick.cycle);

    vga_print(&v, ", tk:");
    vga_print_uint(&v, kGlobal.timing.tick.tick);

    vga_print(&v, ", heap:");
    vga_print_uint(&v, kGlobal.heap.debugInfo.activeBytes);
    vga_print_char(&v, '/');
    vga_print_uint(&v, KERNEL_HEAP_SIZE);
}
