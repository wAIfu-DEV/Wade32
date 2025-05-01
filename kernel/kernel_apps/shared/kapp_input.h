#pragma once

#include "../../types/kapp_inputbuff_type.h"
#include "../../kernel_globals.h"
#include "../../kernel_errors.h"
#include "../../kernel_panic.h"
#include "../../random.h"

void __kapp_keypress_callback(KappInputBuff* ib, const i8 c)
{
    if (ib->buffHead >= KAPP_INPUT_BUFF_SIZE)
        ib->buffHead = 0;

    ib->buff[ib->buffHead] = c;
    ++ib->buffHead;
}

void kapp_request_input_events_custom_callback(KappInputBuff* ib, void (*callback)(KappInputBuff*, const i8))
{
    if (kGlobal.keyboard.inputListenersHead >= KEY_LISTENERS_SIZE)
        kernel_panic(KERR_INPUT_LISTENERS_OVERFLOW, "Too many input listeners.");
    
    u32 id = pseudorandom();
    ib->id = id;

    KappInputListener listener = (KappInputListener){
        .callback = callback,
        .inputBuff = ib,
    };

    // Find empty spot
    if (kGlobal.keyboard.inputListenersHead > 0)
    {
        for (u32 i = 0; i < kGlobal.keyboard.inputListenersHead; ++i)
        {
            KappInputListener* il = &kGlobal.keyboard.inputListeners[i];
            if (il->inputBuff == NULL)
            {
                kGlobal.keyboard.inputListeners[i] = listener;
                return;
            }
        }
    }

    kGlobal.keyboard.inputListeners[kGlobal.keyboard.inputListenersHead] = listener;
    ++kGlobal.keyboard.inputListenersHead;
}

void kapp_request_input_events(KappInputBuff* ib)
{
    kapp_request_input_events_custom_callback(ib, __kapp_keypress_callback);
}

void kapp_remove_input_events(KappInputBuff* ib)
{
    u32 seekId = ib->id;
    for (u32 i = 0; i < kGlobal.keyboard.inputListenersHead; ++i)
    {
        KappInputListener* il = &kGlobal.keyboard.inputListeners[i];
        u32 id = il->inputBuff->id;
        
        if (seekId == id)
        {
            il->inputBuff = NULL;
            return;
        }
    }
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
