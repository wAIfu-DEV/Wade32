#pragma once

#include "cpu/isr.h"
#include "types/extended_tick.h"
#include "kernel_globals.h"

void kernel_tick_update(registers_t regs)
{
    (void)regs;
    extick_add_ticks(&kGlobal.timing.tick, 1); // Increment internal clock

    // Update other tick-based systems (context switching, etc)
}
