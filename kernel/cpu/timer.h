#pragma once

// From: https://github.com/cfenollosa/os-tutorial

#include "isr.h"
#include "../drivers/bios_io.h"
#include "../types/extended_tick.h"
#include "../kernel_globals.h"
#include "../kernel_tick_update.h"

void timer_init(u32 freq)
{
    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, kernel_tick_update);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    u32 divisor = 1193180 / freq;
    u8 low  = (u8)(divisor & 0xFF);
    u8 high = (u8)((divisor >> 8) & 0xFF);
    /* Send the command */
    bios_outb(0x43, 0x36); /* Command port */
    bios_outb(0x40, low);
    bios_outb(0x40, high);
}
