#pragma once
#include "ints.h"


///// string utilities: /////

/// convert integer to string
#define ITOA_SIZE 32
u32 itoa (char* out, i32 n, u32 base);

/// convert string to integer
i32 atoi (const char* in, char** new_pos, u32 base);

/// write formatted text to stream
///   {}  = string
///   {d} = decimal
///   {x} = hexidecimal
///   {p} = pointer
void writef (const char* fmt, ...);

/// stream function for writef
extern void (*writef_output) (char*);

/// make writef write to character buffer
void set_writef_output_buffer (char* buf, u32 cap);
