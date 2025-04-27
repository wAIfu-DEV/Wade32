#pragma once

// From: https://github.com/cfenollosa/os-tutorial

#include "isr.h"
#include "../bios_io.h"
#include "../extended_tick.h"
#include "../kernel_globals.h"

static void timer_callback(registers_t regs) {
    extick_add_ticks(&kGlobal.timing.tick, 1);
}

void init_timer(u32 freq) {
    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    u32 divisor = 1193180 / freq;
    u8 low  = (u8)(divisor & 0xFF);
    u8 high = (u8)((divisor >> 8) & 0xFF);
    /* Send the command */
    bios_outb(0x43, 0x36); /* Command port */
    bios_outb(0x40, low);
    bios_outb(0x40, high);
}
