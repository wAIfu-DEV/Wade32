#pragma once

#include "../../xstd/xstd_core.h"
#include "../types/time_type.h"

u8 rtc_read_register(const u8 reg) {
    bios_outb(0x70, reg);
    return bios_inb(0x71);
}

ibool rtc_is_updating(void) {
    bios_outb(0x70, 0x0A);
    return (ibool)(bios_inb(0x71) & 0x80);
}

u8 bcd_to_binary(const u8 bcd) {
    return (u8)(((bcd >> 4) * 10) + (bcd & 0x0F));
}

Time rtc_get_time(void)
{
    return (Time){
        .seconds = bcd_to_binary(rtc_read_register(0x00)),
        .minutes = bcd_to_binary(rtc_read_register(0x02)),
        .hours =   bcd_to_binary(rtc_read_register(0x04)),
        .day =     bcd_to_binary(rtc_read_register(0x07)),
        .month =   bcd_to_binary(rtc_read_register(0x08)),
        .year =    bcd_to_binary(rtc_read_register(0x09)),
        .century = bcd_to_binary(rtc_read_register(0x32)),
    };
}
