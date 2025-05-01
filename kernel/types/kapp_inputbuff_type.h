#pragma once

#include "../../xstd/xstd_core.h"

#define KAPP_INPUT_BUFF_SIZE 32

typedef struct
{
    u32 id;
    volatile i8 buff[KAPP_INPUT_BUFF_SIZE];
    volatile u8 buffHead;
    volatile u8 consumed;
} KappInputBuff;

typedef void (*__KappKeyCb)(KappInputBuff *ib, i8 c);

typedef struct
{
    __KappKeyCb callback;
    KappInputBuff* inputBuff;
} KappInputListener;

