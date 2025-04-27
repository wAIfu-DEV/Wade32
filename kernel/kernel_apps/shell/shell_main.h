#pragma once

#include "../../../xstd/xstd_writer.h"
#include "../../kapp.h"

Error kapp_shell(void)
{
    // Get subscreen buffer from kernel
    ResultKASB sbRes = kapp_request_screen_buffer((Rectu8){
        .x = 0,
        .y = 0,
        .height = 24,
        .width = 80,
    }, false);

    if (sbRes.error)
        return sbRes.error;
    
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
        return gbWriterRes.error;
    
    GrowBuffWriter gbWriter = gbWriterRes.value;
    Writer* writer = (Writer*)&gbWriter;

    kapp_screen_write_str(&sb, "Wade32 Kernel Shell - 2025\n");
    kapp_screen_write_str(&sb, "shell> ");
    kapp_flush_screen_buffer(&sb);

    Vec2u8 baseOffset = sb.cursor;

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
            if (gbWriter.writeHead)
            {
                c = 0;
                // Should replace with moving rest of buffer -1
                --gbWriter.writeHead;
                Error err = gbWriter.write(&gbWriter, ' ');
                if (err) return err;
            }
            break;
        }
        
        case '\n': {
            c = 0;
            Error err = writer_write_byte(writer, 0);
            if (err) return err;
            HeapStr commandStr = string_dupe_noresult(&kGlobal.heap.allocator, gbWriter.buff.bytes);
            if (!commandStr) return ERR_OUT_OF_MEMORY;
            --gbWriter.writeHead;
            // Handle command string
            break;
        }
        
        default:
            break;
        }

        if (c != 0)
        {
            Error err = writer_write_byte(writer, c);
            if (err) return err;
            // null terminate buffer
            err = writer_write_byte(writer, 0);
            if (err) return err;
        }

        sb.cursor = baseOffset;
        kapp_screen_write_str(&sb, gbWriter.buff.bytes);

        // next write will overwrite null byte
        if (c != 0) --gbWriter.writeHead;
        kapp_flush_screen_buffer(&sb);

        kapp_yield();
    }

    kapp_flush_screen_buffer(&sb);
    kapp_screen_buffer_deinit(&sb);
    return ERR_OK;
}
