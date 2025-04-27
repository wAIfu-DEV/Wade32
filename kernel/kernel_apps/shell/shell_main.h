#pragma once

#include "../../kapp.h"

Error kapp_shell_main(void)
{
    ResultKASB sbRes = kapp_request_screen_buffer((Rectu8){
        .x = 0,
        .y = 0,
        .height = 24,
        .width = 80,
    }, false);

    if (sbRes.error)
        return sbRes.error;
    
    KappScreenBuffer sb = sbRes.value;

    kapp_screen_set_vga_style(&sb, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    kapp_screen_clear(&sb);

    kapp_flush_screen_buffer(&sb);
    return ERR_OK;
}
