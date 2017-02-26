#pragma once
#include "ints.h"

///// string utilities: /////

inline u32 str_len (const char* s)
{
    for (u32 i = 0; ; i++)
        if (!s[i]) return i;
}

inline void str_copy (char* dst, const char* src)
{
    while (*src)
        *dst++ = *src++;
    *dst = '\0';
}

u32 str_copy_cap (char* dst, u32 cap, const char* src);



///// memory utilities /////

void mem_copy (void* dst, void* src, u32 size);
void mem_clear (void* dst, u32 size);
void mem_fill (void* dst, u32 size, u8 with);



///// string parsing and formatting ////

/// convert integer to string
#define ITOA_SIZE 32
u32 itoa (char* out, i32 n, u32 base);
u32 itoau (char* out, u32 n, u32 base);

/// convert string to integer
i32 atoi (const char* in, char** new_pos, u32 base);
u32 atoiu (const char* in, char** new_pos, u32 base);

/// write formatted text to stream
///   {}  = string
///   {d} = decimal
///   {x} = hexidecimal
///   {p} = pointer
///   {*u} = unsigned
void writef (const char* fmt, ...);

/// stream function for writef
extern void (*writef_output) (char*);

/// make writef write to character buffer
void set_writef_output_buffer (char* buf, u32 cap);
