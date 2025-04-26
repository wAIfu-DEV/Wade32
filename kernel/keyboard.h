#pragma once

// Partially AI Generated

#include "../xstd/xstd_core.h"
#include "bios_io.h"
#include "cpu/isr.h"
#include "kernel_globals.h"

#define SCANCODE_TO_ASCII_SIZE 128
const i8 scancode_to_ascii[SCANCODE_TO_ASCII_SIZE] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0, '\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ',
};

void keyboard_handler(registers_t regs) {
    (void)regs;

    u8 scancode = bios_inb(0x60);

    if (scancode & 0x80) {
        // Key released
    } else {
        if (scancode >= SCANCODE_TO_ASCII_SIZE) return;

        char c = scancode_to_ascii[scancode];
        if (c)
        {
            if (kGlobal.keyboard.inputBufferHead >= KEYBOARD_INPUT_BUFF_SIZE) return;
            kGlobal.keyboard.inputBuffer[kGlobal.keyboard.inputBufferHead] = c;
            ++kGlobal.keyboard.inputBufferHead;
        }
    }
}
