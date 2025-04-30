#pragma once

#include "../../../xstd/xstd_writer.h"
#include "../shared/kapp.h"
#include "../../types/args.h"

void __shell_fill_input_buffer(HeapBuff buff, i8 fill)
{
    u32 rest = buff.size;

    while (rest--)
    {
        *buff.bytes = fill;
    }
}

ibool __shell_handle_command(HeapStr commandStr, KappScreenBuffer* sb)
{
    ibool ret = false;
    Allocator* a = &kGlobal.heap.allocator;

    ResultArgs argsRes = args_parse(commandStr, a);
    if (argsRes.error)
    {
        kapp_screen_write_str(sb, "Failed to parse command arguments.");
        goto cleanup;
    }

    Args kappArgs = argsRes.value;

    if (kappArgs.size == 0)
    {
        goto cleanup;
    }

    // Handle command string
    if (string_equals(kappArgs.strings[0], "quit"))
    {
        ret = true;
        goto cleanup;
    }

    kapp_screen_write_char(sb, '\n');

    ResultKEP kepRes = kapp_get_entrypoint(kappArgs.strings[0]);
    if (kepRes.error)
    {
        kapp_screen_write_str(sb, "Could not find kernel app: ");
        kapp_screen_write_str(sb, kappArgs.strings[0]);
    }
    else
    {
        KappEntrypoint kapp = kepRes.value;
        KappReturn kappRet = kapp(kappArgs);

        if (kappRet.outOrNull)
        {
            kapp_screen_write_str(sb, kappRet.outOrNull);
            a->free(a, kappRet.outOrNull);
        }

        if (kappRet.errcode)
        {
            if (kappRet.outOrNull)
            {
                kapp_screen_write_str(sb, "\n");
            }

            kapp_screen_write_str(sb, "Failed with error: ");
            kapp_screen_write_str(sb, ErrorToString(kappRet.errcode));
        }
    }

cleanup:
    while (false); // labels are stupid

    for (u32 i = 0; i < kappArgs.size; ++i)
    {
        a->free(a, (HeapStr)kappArgs.strings[i]);
    }
    a->free(a, kappArgs.strings);
    return ret;
}

KappReturn kapp_shell(Args args)
{
    (void)args;
    Error err;

    // Get subscreen buffer from kernel
    ResultKASB sbRes = kapp_request_screen_buffer((Rectu8){
        .x = 1,
        .y = 0,
        .height = 23,
        .width = 78,
    }, false);

    if (sbRes.error)
        return (KappReturn){
            .errcode = sbRes.error,
            .outOrNull = NULL,
        };
    
    KappScreenBuffer sb = sbRes.value;

    // Clear screen
    kapp_screen_set_vga_style(&sb, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    kapp_screen_clear(&sb);
    // Apply changes to video memory
    kapp_flush_screen_buffer(&sb);

    // Create input buffer
    KappInputBuff inputBuff = (KappInputBuff){
        .buffHead = 0,
        .consumed = 0,
    };

    // Get key updates from kernel
    kapp_request_input_events(&inputBuff);

    ResultGrowBuffWriter gbWriterRes = growbuffwriter_init(kGlobal.heap.allocator, 32);
    if (gbWriterRes.error)
    {
        err = gbWriterRes.error;
        goto cleanup;
    }
    
    GrowBuffWriter gbWriter = gbWriterRes.value;
    Writer* writer = (Writer*)&gbWriter;

    kapp_screen_write_str(&sb, "Wade32 Kernel Shell - 2025\n");
    kapp_screen_write_str(&sb, "shell> ");
    kapp_flush_screen_buffer(&sb);

    while (true)
    {
        i8 c = kapp_input_get_key(&inputBuff);
            
        if (c == 0)
        {
            kapp_yield();
            continue;
        }

        switch (c)
        {
        case '\b': {
            c = 0;
            if (gbWriter.writeHead > 0)
            {
                // Should replace with moving rest of buffer -1
                --gbWriter.writeHead;
                err = writer_write_byte(writer, 0);
                if (err)
                    goto cleanup;
            }
            break;
        }
        
        case '\n': {
            c = 0;
            err = writer_write_byte(writer, 0);
            if (err)
                goto cleanup;
            
            HeapStr commandStr = string_dupe_noresult(&kGlobal.heap.allocator, gbWriter.buff.bytes);
            if (!commandStr)
            {
                err = ERR_OUT_OF_MEMORY;
                goto cleanup;
            }
            --gbWriter.writeHead;

            ibool shouldQuit = __shell_handle_command(commandStr, &sb);
            if (shouldQuit)
                goto cleanup;

            // Clear allocated memory
            kGlobal.heap.allocator.free(&kGlobal.heap.allocator, commandStr);
            growbuffwriter_resize(&gbWriter, 32);
            __shell_fill_input_buffer(gbWriter.buff, 0);

            kapp_screen_write_str(&sb, "\nshell> ");

            // Reset writer
            gbWriter.writeHead = 0;
            kapp_flush_screen_buffer(&sb);
            continue;
        }
        
        default:
            break;
        }

        if (c != 0)
        {
            err = writer_write_byte(writer, c);
            if (err)
                goto cleanup;
            // null terminate buffer
            err = writer_write_byte(writer, 0);
            if (err)
                goto cleanup;
        }

        if (gbWriter.writeHead > 0)
        {
            u32 bound = c != 0 ? gbWriter.writeHead - 2 : gbWriter.writeHead;
            for (u32 i = 0; i < bound; ++i)
                __kapp_screen_retreat_cursor(&sb);
        }
        
        kapp_screen_write_str(&sb, gbWriter.buff.bytes);

        // next write will overwrite null byte
        if (gbWriter.writeHead > 0)
            --gbWriter.writeHead;
        kapp_flush_screen_buffer(&sb);

        kapp_yield();
    }

cleanup:
    kapp_screen_set_vga_style(&sb, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kapp_screen_clear(&sb);
    kapp_flush_screen_buffer(&sb);
    kapp_screen_buffer_deinit(&sb);
    return (KappReturn){
        .errcode = ERR_OK,
        .outOrNull = NULL,
    };
}
