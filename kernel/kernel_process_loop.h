#pragma once

#include "vga.h"
#include "kernel_globals.h"
#include "scheduler.h"

void kernel_process(void)
{
    VgaInterface* vga = &kGlobal.screen.vga;
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
