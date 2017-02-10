#pragma once
#include <stdint.h>

/// convert integer to string
#define ITOA_SIZE 32
char* itoa (char* out, int n, int base);

/// convert string to integer
int atoi (const char* in, char** new_pos, int base);

/// write formatted text to stream
///   {}  = string
///   {d} = decimal
///   {x} = hexidecimal
///   {p} = pointer
void writef (const char* fmt, ...);

/// stream function for writef
extern void (*writef_output) (char*);
