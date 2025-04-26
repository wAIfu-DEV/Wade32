#pragma once

// From https://github.com/cfenollosa/os-tutorial

#include "../xstd/xstd_core.h"

u8 bios_inb(u16 port) {
    u8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void bios_outb(u16 port, u8 value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
