#pragma once

#include "../../xstd/xstd_error.h"

#include "args.h"

typedef struct
{
    i8* outOrNull;
    Error errcode;
} KappReturn;

typedef KappReturn (*KappEntrypoint)(Args);
