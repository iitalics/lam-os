#pragma once
#include <misc/ints.h>
#include <arch_constants.h>

// notify memory management of a new region of available RAM
void memory_ram_avail (uptr start, uptr end);

// panic if out of memory?
extern int panic_on_oom;

// allocate arbitrary chunk
void* k_alloc (uptr size);

// free allocated memory
void k_free (void* ptr);
