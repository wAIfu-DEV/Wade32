#pragma once

#include "../../xstd/xstd_core.h"
#include "../cpu/interrupts.h"

u32 disk_load(u8 numSectors, u8 drive, u8 startSector, u16 writeToSegment, u16 writeToOffset)
{
    u8 result;
    __asm__ volatile(
        "push %%ax\n\t"
        "push %%bx\n\t"
        "push %%cx\n\t"
        "push %%dx\n\t"
        "push %%es\n\t"

        "mov %4, %%es\n\t"       // Set ES (buffer offset segment)
        "mov %5, %%bx\n\t"       // Set BX (buffer offset)
        "mov $0x02, %%ah\n\t"    // AH = 0x02 (read sectors)
        "mov %1, %%al\n\t"       // AL = number of sectors to read
        "mov %3, %%cl\n\t"       // CL = start sector (1–63)
        "mov $0x00, %%ch\n\t"    // CH = cylinder
        "mov $0x00, %%dh\n\t"    // DH = head
        "mov %2, %%dl\n\t"       // DL = drive
        "int $0x13\n\t"          // BIOS interrupt to read disk
        "jc disk_fail\n\t"       // If carry set, error

        "cmp %1, %%al\n\t"       // Compare sectors read to requested
        "jne sectors_mismatch\n\t"

        "mov $0, %0\n\t"         // result = 0 (success)
        "jmp done\n\t"

        "disk_fail:\n\t"
        "mov $1, %0\n\t"         // result = 1 (BIOS error)
        "jmp done\n\t"

        "sectors_mismatch:\n\t"
        "mov $2, %0\n\t"         // result = 2 (partial read)

        "done:\n\t"
        "pop %%es\n\t"
        "pop %%dx\n\t"
        "pop %%cx\n\t"
        "pop %%bx\n\t"
        "pop %%ax\n\t"
        : "=r"(result)                    // output
        : "r"(numSectors),                // %1
          "r"(drive),                     // %2
          "r"(startSector),               // %3
          "r"(writeToSegment),            // %4
          "r"(writeToOffset)              // %5
        : "ax", "bx", "cx", "dx", "memory"
    );
    return result;
}
