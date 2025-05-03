#pragma once

#include "../../xstd/xstd_core.h"
#include "../../xstd/xstd_writer.h"
#include "../kernel_globals.h"

typedef struct
{
    u32 size;
    ConstStr* strings;
} Args;

typedef struct
{
    Args value;
    Error error;
} ResultArgs;

ResultArgs args_parse(ConstStr command, Allocator *alloc)
{
    ResultList argListRes = list_init(alloc, sizeof(HeapStr), 4);
    if (argListRes.error)
        return (ResultArgs){
            .error = argListRes.error,
        };

    List argList = argListRes.value;

    ResultGrowStrWriter gswRes = growstrwriter_init(*alloc, 4);
    if (gswRes.error)
        return (ResultArgs){
            .error = gswRes.error,
        };
    
    GrowStrWriter gsw = gswRes.value;

    u32 bound = string_size(command);    

    struct flags
    {
        ibool inString;
    } parseFlags = {
        .inString = false,
    };

    for (u32 i = 0; i < bound; ++i)
    {
        i8 c = command[i];

        if (c == '"')
        {
            parseFlags.inString = !parseFlags.inString;
            continue;
        }

        else if (c == ' ' && gsw.writeHead > 0 && !parseFlags.inString)
        {
            // move string to arg list
            list_push(&argList, &gsw.str);

            // Replace writer
            gswRes = growstrwriter_init(*alloc, 8);
            if (gswRes.error)
                return (ResultArgs){
                    .error = gswRes.error,
                };

            gsw = gswRes.value;
        }

        else
        {
            Writer* writer = (Writer*)&gsw;
            writer_write_byte(writer, c);
        }
    }

    if (gsw.writeHead > 0)
    {
        // move string to arg list
        list_push(&argList, &gsw.str);
    } else {
        alloc->free(alloc, gsw.str);
    }

    if (parseFlags.inString)
    {
        return (ResultArgs){
            .error = ERR_PARSE_ERROR,
        };
    }

    return (ResultArgs){
        .value = (Args){
            .strings = (ConstStr*)argList._data,
            .size = argList._itemCnt,
        },
        .error = ERR_OK,
    };
}
