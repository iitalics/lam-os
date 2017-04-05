#pragma once
#include <misc/ints.h>
#include <arch_constants.h>

// notify memory management of a new region of available RAM
void memory_ram_avail (uptr start, uptr end);

// panic if out of memory?
extern int panic_on_oom;

// align to?
#define KALLOC_ALIGN_TO 8

// ask kernel for memory permanently
void* k_ask_permanent (uptr* size_in_out);

// allocate arbitrary chunk
void* k_alloc (uptr size)
    __attribute__
    ((assume_aligned(KALLOC_ALIGN_TO, sizeof(uptr))));

// free allocated memory
void k_free (void* ptr);
