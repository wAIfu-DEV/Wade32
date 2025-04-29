#pragma once

void interrupts_disable()
{
    __asm__ volatile("cli");
}

void interrupts_enable()
{
    __asm__ volatile("sti");
}
