#pragma once

#include "../../../xstd/xstd_writer.h"
#include "../shared/kapp.h"
#include "../../types/args.h"
#include "../../kernel_breakpoint.h"

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
        goto cleanup; // Empty input is valid user input, should just skip
                      // to new line.

    // Explicit quit user input
    if (string_equals(kappArgs.strings[0], "quit"))
    {
        ret = true;
        goto cleanup;
    }

    kapp_screen_write_char(sb, '\n');

    // Fetch entrypoint from kapp registry
    ResultKEP kepRes = kapp_get_entrypoint(kappArgs.strings[0]);
    if (kepRes.error)
    {
        kapp_screen_write_str(sb, "Could not find kernel app: ");
        kapp_screen_write_str(sb, kappArgs.strings[0]);
    }
    else
    {
        KappEntrypoint kapp = kepRes.value;
        KappReturn kappRet = kapp(kappArgs); // Call kapp

        // Print stdout 
        if (kappRet.outOrNull)
        {
            kapp_screen_write_str(sb, kappRet.outOrNull);
            a->free(a, kappRet.outOrNull);
        }

        if (kappRet.errcode)
        {
            if (kappRet.outOrNull)
                kapp_screen_write_str(sb, "\n");
            
            kapp_screen_write_str(sb, "Failed with error: ");
            kapp_screen_write_str(sb, ErrorToString(kappRet.errcode));
        }
    }

cleanup:
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
    Error err = ERR_OK;

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
    kapp_flush_screen_buffer(&sb); // Apply changes to video memory

    // Create input buffer
    KappInputBuff inputBuff = (KappInputBuff){
        .buffHead = 0,
        .consumed = 0,
    };

    // Get key updates from kernel
    kapp_request_input_events(&inputBuff);

    ResultGrowStrWriter gswRes = growstrwriter_init(kGlobal.heap.allocator, 8);
    if (gswRes.error)
    {
        err = gswRes.error;
        goto cleanup;
    }
    
    GrowStrWriter gsw = gswRes.value;
    Writer* writer = (Writer*)&gsw;

    kapp_screen_write_str(&sb, "Wade32 Kernel Shell - 2025\n");
    kapp_screen_write_str(&sb, "shell> ");
    kapp_flush_screen_buffer(&sb);

    while (true)
    {
        i8 c = kapp_input_get_key(&inputBuff);
        
        if (c == 0)
        {
            kapp_yield(); // Yield compute time to kernel routines
            continue;
        }

        switch (c)
        {
        case '\b': {
            c = 0;
            if (gsw.writeHead > 0)
            {
                // Replace currently printed character with space
                __kapp_screen_retreat_cursor(&sb);
                kapp_screen_write_char(&sb, ' ');
                __kapp_screen_retreat_cursor(&sb);

                // Overwrite end of user input
                --gsw.writeHead;
                gsw.str[gsw.writeHead] = 0;
            }
            break;
        }
        
        case '\n': {
            c = 0;
            HeapStr commandStr = gsw.str;

            ibool shouldQuit = __shell_handle_command(commandStr, &sb);
            if (shouldQuit)
                goto cleanup;

            // Reset writer
            gsw.writeHead = 0;
            growstrwriter_resize(&gsw, 8);
            gsw.str[0] = 0;

            kapp_screen_write_str(&sb, "\nshell> ");
            kapp_flush_screen_buffer(&sb);
            continue;
        }
        
        default:
            break;
        }

        if (c != 0)
        {
            // Write character to user input buffer
            err = writer_write_byte(writer, c);
            if (err) goto cleanup;
        }

        if (gsw.writeHead > 0)
        {
            // Move cursor to start of line before printing updated user input
            u32 bound = c != 0 ? gsw.writeHead - 1 : gsw.writeHead;
            for (u32 i = 0; i < bound; ++i)
                __kapp_screen_retreat_cursor(&sb);
        }
        
        kapp_screen_write_str(&sb, gsw.str);
        kapp_flush_screen_buffer(&sb);

        kapp_yield(); // Yield compute time to kernel routines
    }

cleanup:
    kapp_remove_input_events(&inputBuff);
    kapp_screen_set_vga_style(&sb, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kapp_screen_clear(&sb);
    kapp_flush_screen_buffer(&sb);
    kapp_screen_buffer_deinit(&sb);

    return (KappReturn){
        .errcode = err,
        .outOrNull = NULL,
    };
}
