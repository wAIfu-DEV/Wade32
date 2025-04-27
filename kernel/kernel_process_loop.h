#pragma once

#include "vga.h"
#include "kernel_globals.h"
#include "scheduler.h"

void kernel_process(void)
{
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

    __asm__ volatile("hlt");
}
