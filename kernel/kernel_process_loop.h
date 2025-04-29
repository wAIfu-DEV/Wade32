#pragma once

#include "cpu/halt.h"
#include "cpu/interrupts.h"

#include "vga_interface.h"
#include "kernel_globals.h"
#include "scheduler.h"

/**
 * @brief "Main loop" of the kernel, handling user input and scheduler processing.
 */
void kernel_process(void)
{
    interrupts_disable(); // Critical section

    // Handle keyboard input
    if (kGlobal.keyboard.inputBufferHead > 0)
    {
        for (u32 i = 0; i < kGlobal.keyboard.inputBufferHead; ++i)
        {
            i8 c = kGlobal.keyboard.inputBuffer[i];
            for (u32 j = 0; j < kGlobal.keyboard.inputListenersHead; ++j)
            {
                KappInputListener listener = kGlobal.keyboard.inputListeners[j];
                listener.callback(listener.inputBuff, c);
            }
        }
        kGlobal.keyboard.inputBufferHead = 0;
    }

    // Handle scheduled events
    scheduler_process();

    interrupts_enable(); // Make sure interrupts are enabled
                         // if not will lead to hang
    halt();
}
