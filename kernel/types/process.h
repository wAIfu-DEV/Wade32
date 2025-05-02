#pragma once

#include "extended_id.h"
#include "extended_tick.h"
#include "proc_context.h"
#include "proc_priority.h"
#include "proc_readystate.h"

typedef struct
{
    Context context;
    ExID id;
    ExTick startTime;
    ProcessPriority priority;
    ProcessReadyState readyState;
} Process;
