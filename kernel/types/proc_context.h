#pragma once

#include "../../xstd/xstd_core.h"

typedef struct {
    u32 edi, esi, ebp;
    u32 esp;      // saved stack pointer
    u32 ebx, edx, ecx, eax;
    u32 eip;      // saved instruction pointer
    u32 eflags;   // saved flags
} Context;
