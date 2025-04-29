#pragma once

#include "bios_io.h"
#include "../cpu/halt.h"

void shutdown(void)
{
    bios_outw(0x604, 0x2000); // Try the ACPI shutdown port
    bios_outw(0xB004, 0x2000); // Alternative shutdown (older QEMU, Bochs)
    
    while(true)
    {
        halt();
    }
}
