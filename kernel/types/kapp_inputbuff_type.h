#pragma once

#include "../../xstd/xstd_core.h"
#include "../types/extended_id.h"

#define KAPP_INPUT_BUFF_SIZE 32

typedef struct
{
    ExID id;
    i8 buff[KAPP_INPUT_BUFF_SIZE];
    u8 buffHead;
    u8 consumed;
} KappInputBuff;

typedef void (*__KappKeyCb)(KappInputBuff *ib, i8 c);

typedef struct
{
    __KappKeyCb callback;
    KappInputBuff* inputBuff;
    ibool valid;
} KappInputListener;

