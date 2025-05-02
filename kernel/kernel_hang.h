#pragma once

#include "kernel_process_loop.h"
#include "kernel_apps/shared/kapp_input.h"
#include "kernel_process_keyboard.h"

void kernel_hang(void)
{
    interrupts_disable();

    while (true)
        halt_enable_ints();
}

void kernel_hang_until_keypress(void)
{
    KappInputBuff ib = (KappInputBuff){
        .buffHead = 0,
        .consumed = 0,
    };
    kapp_request_input_events(&ib);

    while (kapp_input_get_key(&ib) == 0)
    {
        interrupts_disable();
        keyboard_process();
        halt_enable_ints();
    }
    kapp_remove_input_events(&ib);
}

void kernel_process_until_keypress(void)
{
    KappInputBuff ib = (KappInputBuff){
        .buffHead = 0,
        .consumed = 0,
    };
    kapp_request_input_events(&ib);

    while (kapp_input_get_key(&ib) == 0)
    {
        kernel_process();
    }
    kapp_remove_input_events(&ib);
}
