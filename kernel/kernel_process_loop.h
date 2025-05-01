#pragma once

#include "cpu/halt.h"
#include "cpu/interrupts.h"

#include "vga_interface.h"
#include "kernel_globals.h"
#include "scheduler.h"
#include "kernel_process_keyboard.h"

/**
 * @brief "Main loop" of the kernel, handling user input and scheduler processing.
 */
void kernel_process(void)
{
    interrupts_disable(); // Critical section

    // Handle keyboard input
    keyboard_process();

    // Handle scheduled events
    scheduler_process();

    halt();
}
