#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_alloc.h"

typedef struct _writer
{
    Error (*write)(void* writer, i8 byte);
    u64 writeHead;
    u64 writeEnd;
} Writer;

typedef struct _buff_writer
{
    Error (*write)(struct _buff_writer* w, i8 byte);
    u64 writeHead;
    u64 writeEnd;
    Buffer buff;
} BuffWriter;

typedef struct _grow_buff_writer
{
    Error (*write)(struct _grow_buff_writer* w, i8 byte);
    u64 writeHead;
    u64 writeEnd;
    HeapBuff buff;
    Allocator allocator;
} GrowBuffWriter;

typedef struct _grow_str_writer
{
    Error (*write)(struct _grow_str_writer* w, i8 byte);
    u64 writeHead;
    u64 writeEnd;
    HeapStr str;
    u32 strSize;
    Allocator allocator;
} GrowStrWriter;

typedef struct _result_buff_writer
{
    BuffWriter value;
    Error error;
} ResultBuffWriter;

typedef struct _result_grow_buff_writer
{
    GrowBuffWriter value;
    Error error;
} ResultGrowBuffWriter;

typedef struct _result_grow_str_writer
{
    GrowStrWriter value;
    Error error;
} ResultGrowStrWriter;


Error buffwriter_write_byte(BuffWriter* bw, i8 byte)
{
    if (!bw)
        return ERR_INVALID_PARAMETER;

    if (bw->writeHead >= bw->writeEnd)
        return ERR_WOULD_OVERFLOW;
    
    bw->buff.bytes[bw->writeHead] = byte;
    ++bw->writeHead;
    return ERR_OK;
} 

Error growbuffwriter_write_byte(GrowBuffWriter* gbw, i8 byte)
{
    if (!gbw)
        return ERR_INVALID_PARAMETER;

    // Grow buffer
    if (gbw->writeHead >= gbw->writeEnd)
    {
        u64 newSize = gbw->buff.size * 2;

        if (newSize < gbw->buff.size)
            return ERR_WOULD_OVERFLOW;

        i8* newBlock = gbw->allocator.realloc(&gbw->allocator, gbw->buff.bytes, newSize);

        if (!newBlock)
            return ERR_OUT_OF_MEMORY;
        
        gbw->buff.bytes = newBlock;
        gbw->buff.size = newSize;
        gbw->writeEnd = newSize;
    }
    
    gbw->buff.bytes[gbw->writeHead] = byte;
    ++gbw->writeHead;
    return ERR_OK;
}

Error growstrwriter_write_byte(GrowStrWriter* gsw, i8 byte)
{
    if (!gsw)
        return ERR_INVALID_PARAMETER;

    // Grow buffer
    if (gsw->writeEnd == 0 || gsw->writeHead >= gsw->writeEnd - 1)
    {
        u64 newSize = gsw->strSize * 2;

        if (newSize < gsw->strSize)
            return ERR_WOULD_OVERFLOW;

        i8* newBlock = gsw->allocator.realloc(&gsw->allocator, gsw->str, newSize);

        if (!newBlock)
            return ERR_OUT_OF_MEMORY;
        
        gsw->str = newBlock;
        gsw->strSize = newSize;
        gsw->writeEnd = newSize;
    }
    
    gsw->str[gsw->writeHead] = byte;
    gsw->str[gsw->writeHead+1] = 0;
    ++gsw->writeHead;
    return ERR_OK;
} 

ResultBuffWriter buffwriter_init(Buffer buff)
{
    if (buff.bytes == NULL || buff.size == 0)
        return (ResultBuffWriter){
            .error = ERR_INVALID_PARAMETER,
        };
    
    return (ResultBuffWriter){
        .value = (BuffWriter){
            .buff = buff,
            .writeHead = 0,
            .writeEnd = buff.size,
            .write = buffwriter_write_byte,
        },
        .error = ERR_OK,
    };
}

ResultGrowBuffWriter growbuffwriter_init(Allocator alloc, u32 initSize)
{
    if (initSize == 0)
        return (ResultGrowBuffWriter){
            .error = ERR_INVALID_PARAMETER,
        };
    
    i8* block = alloc.alloc(&alloc, initSize);

    if (!block)
        return (ResultGrowBuffWriter){
            .error = ERR_OUT_OF_MEMORY,
        };
    
    return (ResultGrowBuffWriter){
        .value = (GrowBuffWriter){
            .buff = (HeapBuff){
                .bytes = block,
                .size = initSize,
            },
            .allocator = alloc,
            .writeHead = 0,
            .writeEnd = initSize,
            .write = growbuffwriter_write_byte,
        },
        .error = ERR_OK,
    };
}

ResultGrowStrWriter growstrwriter_init(Allocator alloc, u32 initSize)
{
    if (initSize == 0)
        return (ResultGrowStrWriter){
            .error = ERR_INVALID_PARAMETER,
        };
    
    i8* block = alloc.alloc(&alloc, initSize);

    if (!block)
        return (ResultGrowStrWriter){
            .error = ERR_OUT_OF_MEMORY,
        };
    
    return (ResultGrowStrWriter){
        .value = (GrowStrWriter){
            .str = block,
            .strSize = initSize,
            .allocator = alloc,
            .writeHead = 0,
            .writeEnd = initSize,
            .write = growstrwriter_write_byte,
        },
        .error = ERR_OK,
    };
}

Error growbuffwriter_resize(GrowBuffWriter* gbw, u64 newSize)
{
    if (!gbw || newSize == 0)
        return ERR_INVALID_PARAMETER;

    i8* newBlock = gbw->allocator.realloc(&gbw->allocator, gbw->buff.bytes, newSize);

    if (!newBlock)
        return ERR_OUT_OF_MEMORY;
    
    gbw->buff.bytes = newBlock;
    gbw->buff.size = newSize;
    gbw->writeEnd = newSize;
    gbw->writeHead = newSize < gbw->writeHead ? newSize : gbw->writeHead;
    return ERR_OK;
}

Error writer_write_byte(Writer* w, i8 byte)
{
    if (!w)
        return ERR_INVALID_PARAMETER;

    return w->write(w, byte);
}

Error writer_write_null(Writer* w);

Error writer_write_bytes(Writer* w, ConstBuff buff)
{
    if (!w || !buff.bytes)
        return ERR_INVALID_PARAMETER;

    for (u32 i = 0; i < buff.size && w->writeHead < w->writeEnd; ++i)
    {
        Error err = w->write(w, buff.bytes[i]);
        if (err != ERR_OK)
            return err;
    }
    
    return ERR_OK;
}

Error writer_write_str(Writer* w, ConstStr text)
{
    if (!w)
        return ERR_INVALID_PARAMETER;

    if (!text)
        return writer_write_null(w);

    while (*text && w->writeHead < w->writeEnd)
    {
        Error err = w->write(w, *text);
        if (err != ERR_OK)
            return err;
        ++text;
    }
    
    return ERR_OK;
}

Error writer_write_null(Writer* w)
{
    if (!w)
        return ERR_INVALID_PARAMETER;

    return writer_write_str(w, "(null)");
}

Error writer_write_int(Writer* w, i64 i)
{
    if (!w)
        return ERR_INVALID_PARAMETER;

    char buf[20];
    i64 n = i;
    i16 idx = 0;
    Error err;

    if (n == 0)
    {
        err = writer_write_byte(w, '0');
        if (err) return err;
        return ERR_OK;
    }

    if (n < 0)
    {
        err = writer_write_byte(w, '-');
        if (err) return err;
        n = -n;
    }

    while (n != 0)
    {
        i16 d = (i16)(n % 10);
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        err = writer_write_byte(w, buf[j]);
        if (err) return err;
    }
    return ERR_OK;
}

Error writer_write_uint(Writer* w, u64 i)
{
    if (!w)
        return ERR_INVALID_PARAMETER;

    char buf[20];
    i16 idx = 0;
    u64 n = i;
    Error err;

    if (n == 0)
    {
        err = writer_write_byte(w, '0');
        if (err) return err;
        return ERR_OK;
    }

    while (n != 0)
    {
        i16 d = (i16)(n % 10);
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        err = writer_write_byte(w, buf[j]);
        if (err) return err;
    }
    return ERR_OK;
}

Error writer_write_float(Writer* w, f64 flt, u64 precision)
{
    if (!w)
        return ERR_INVALID_PARAMETER;

    f64 d = flt;
    Error err;

    if (d < 0)
    {
        err = writer_write_byte(w, '-');
        if (err) return err;
        d = -d;
    }

    u64 intPart = (u64)d;
    f64 fracPart = d - (f64)intPart;

    err = writer_write_uint(w, intPart);
    if (err) return err;

    if (precision <= 0)
        return ERR_OK;

    err = writer_write_byte(w, '.');
    if (err) return err;

    f64 scale = 1.0;
    for (u64 i = 0; i < precision; ++i)
        scale *= (f64)10.0;

    fracPart *= scale;
    u64 fracInt = (u64)(fracPart + 0.5); // rounding

    // Ensure leading zeros in fractional part
    u64 div = 1;
    for (u64 i = 1; i < precision; ++i)
        div *= 10;

    while (div > fracInt && div > 1)
    {
        err = writer_write_byte(w, '0');
        if (err) return err;
        div /= 10;
    }

    err = writer_write_uint(w, fracInt);
    if (err) return err;
    return ERR_OK;
}
