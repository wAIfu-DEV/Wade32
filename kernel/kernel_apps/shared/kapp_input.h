#pragma once

#include "../../types/kapp_inputbuff_type.h"
#include "../../kernel_globals.h"
#include "../../kernel_errors.h"
#include "../../kernel_panic.h"

void __kapp_keypress_callback(KappInputBuff* ib, const i8 c)
{
    if (ib->buffHead >= KAPP_INPUT_BUFF_SIZE)
        ib->buffHead = 0;

    ib->buff[ib->buffHead] = c;
    ++ib->buffHead;
}

void kapp_request_input_events(KappInputBuff* ib)
{
    if (kGlobal.keyboard.inputListenersHead >= KEY_LISTENERS_SIZE)
        kernel_panic(KERR_INPUT_LISTENERS_OVERFLOW, "Too many input listeners.");
    
    KappInputListener listener = (KappInputListener){
        .callback = __kapp_keypress_callback,
        .inputBuff = ib,
    };

    kGlobal.keyboard.inputListeners[kGlobal.keyboard.inputListenersHead] = listener;
    ++kGlobal.keyboard.inputListenersHead;
}

i8 kapp_input_get_key(KappInputBuff* ib)
{
    if (ib->consumed >= ib->buffHead)
    {
        ib->consumed = 0;
        ib->buffHead = 0;
        return 0;
    }

    i8 c = ib->buff[ib->consumed];
    ++ib->consumed;
    return c;
}
