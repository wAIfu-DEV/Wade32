#pragma once

#include "../xstd/xstd_error.h"

typedef struct
{
    i8* outOrNull;
    Error errcode;
} KappReturn;

typedef KappReturn (*KappEntrypoint)(void);
