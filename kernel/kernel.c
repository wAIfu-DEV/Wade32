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
#include "kernel_header.h"
#include "kernel_hang.h"
#include "kernel_stack_canary.h"

#include "kernel_apps/kapps_reg.h"
#include "kernel_apps/kapps_init.h"
#include "kernel_apps/shared/kapp_exec.h"

#include "random.h"

#include "kernel_page_allocator.h"

void __scheduled_shutdown(void*);
void __scheduled_stack_canary_check(void*);

/**
 * @brief Entry point of the kernel.
 * 
 */
__attribute__((externally_visible, used, noinline, visibility("default")))
void kernel_main(void)
{
    // We should zero out the BSS
    Error err;
    kernel_setup_stack_canary();

    // Create VGA interface
    kGlobal.screen.vga = vga_create_interface();
    VgaInterface* vga = &kGlobal.screen.vga;
    vga_set_style(vga, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear_screen(vga);
    vga_set_cursor_position(vga, 0, 1);

    vga_print(vga, "\n> Kernel loaded.\n");
    vga_print(vga, "\n> Created VGA interface.\n");

    // Create page allocator
    // Allows mapping floppy memory after kernel/BIOS memory
    ResultAllocator pageAllocRes = kernel_create_page_allocator();
    if (pageAllocRes.error)
        kernel_panic(KERR_PAGE_ALLOC_INIT_FAILURE, "Failed to create kernel page allocator.");
    
    kGlobal.memory.pageAlloc = pageAllocRes.value;
    vga_print(vga, "> Created page allocator.\n");

    // Create heap allocator + Debug allocator for info
    kGlobal.heap.heapMem = kGlobal.memory.pageAlloc.alloc(&kGlobal.memory.pageAlloc, KERNEL_HEAP_SIZE);
    Buffer heapBuff = (Buffer){.bytes = (i8*)kGlobal.heap.heapMem, .size = KERNEL_HEAP_SIZE};
    
    ResultAllocator allocRes = buffer_allocator(heapBuff);
    if (allocRes.error)
        kernel_panic(KERR_ALLOC_INIT_FAILURE, "Failed to create kernel heap allocator.");
    
    Allocator heapAlloc = allocRes.value;
    vga_print(vga, "> Created kernel heap allocator.\n");

    ResultAllocator debugAllocRes = debug_allocator(&kGlobal.heap.debugInfo, &heapAlloc);
    if (debugAllocRes.error)
        kernel_panic(KERR_ALLOC_INIT_FAILURE, "Failed to create kernel heap debug allocator.");
    
    kGlobal.heap.allocator = debugAllocRes.value;
    vga_print(vga, "> Created kernel heap debug allocator wrapper.\n");

    // Initialize interrupts
    isr_install();
    interrupts_enable();
    vga_print(vga, "> Initialized interrupts.\n");

    // Create tick timer
    timer_init(KERNEL_TICK_FREQ);
    vga_print(vga, "> Started tick timer.\n");

    // Schedule stack canary checking
    u32 timeout = ms_to_ticks(KERNEL_PRINT_TIME_INTERVAL_MS);
    schedule(timeout, kernel_draw_header, NULL, true);
    vga_print(vga, "> Scheduled timings printing routine.\n");
    
    // Schedule header drawing routine
    timeout = ms_to_ticks(KERNEL_CHECK_CANARY_INTERVAL_MS);
    schedule(timeout, __scheduled_stack_canary_check, NULL, true);
    vga_print(vga, "> Scheduled timings printing routine.\n");

    // Initialize kapp registry
    err = kernel_kappreg_init();
    if (err)
        kernel_panic(KERR_ENV_INIT_ERROR, "Failed to initialize kernel apps registry.");
    
    vga_print(vga, "> Initialized kernel apps registry.\n");

    // Register keyboard input handler
    register_interrupt_handler(IRQ1, keyboard_driver_in_handler);
    vga_print(vga, "> Registered keyboard handler, switching to user input mode.\n");

    // Set random seed
    Time t = time_utc();
    pseudorandom_set_seed((u32)t.seconds + (u32)t.minutes + (u32)t.hours);

    // Fetch shell from kapp registry
    ResultKEP kepRes = kapp_get_entrypoint("shell");
    if (kepRes.error)
        kernel_panic(KERR_NO_SHELL, "Failed to find kernel shell.");
    
    // Call shell
    KappEntrypoint shell = kepRes.value;
    KappReturn ret = shell((Args){0});
    if (ret.errcode)
        kernel_panic((u32)ret.errcode, ErrorToString(ret.errcode));
    
    vga_clear_screen(vga);
    vga_set_cursor_position(vga, 0, 1);
    vga_print(vga, "> Exited shell, shutdown in 3s.\n");
    timeout = ms_to_ticks(3000);
    schedule(timeout, __scheduled_shutdown, NULL, false);

    while (true)
        kernel_process();

    kernel_hang();
}

void __scheduled_shutdown(void* arg0)
{
    (void)arg0;
    kernel_shutdown();
}

void __scheduled_stack_canary_check(void* arg0)
{
    (void)arg0;
    kernel_check_stack_canary();
}
