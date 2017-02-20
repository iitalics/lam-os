#pragma once
#include <misc/ints.h>

extern char* kernel_panic_str;
extern u32 kernel_panic_str_cap;

void kernel_panic () __attribute__((noreturn));

#define kernel_panicf(...) do { \
    set_writef_output_buffer(kernel_panic_str, \
                             kernel_panic_str_cap); \
    writef(__VA_ARGS__); \
    kernel_panic(); \
    } while(0)
