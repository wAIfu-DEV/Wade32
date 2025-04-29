#pragma once

// From https://github.com/cfenollosa/os-tutorial

#include "../../xstd/xstd_core.h"

u8 bios_inb(const u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void bios_outb(const u16 port, const u8 value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

u16 bios_inw(const u16 port)
{
    u16 ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void bios_outw(const u16 port, const u16 value)
{
    __asm__ volatile ( "outw %0, %1" : : "a"(value), "Nd"(port) );
}
