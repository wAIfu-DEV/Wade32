#pragma once

#include "types/kapp_inputbuff_type.h"
#include "kernel_globals.h"

void keyboard_process(void)
{
    if (kGlobal.keyboard.inputBufferHead > 0)
    {
        for (u32 i = 0; i < kGlobal.keyboard.inputBufferHead; ++i)
        {
            i8 c = kGlobal.keyboard.inputBuffer[i];
            for (u32 j = 0; j < kGlobal.keyboard.inputListenersHead; ++j)
            {
                KappInputListener listener = kGlobal.keyboard.inputListeners[j];
                if (listener.valid)
                    listener.callback(listener.inputBuff, c);
            }
        }
        kGlobal.keyboard.inputBufferHead = 0;
    }
}
