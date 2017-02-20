#pragma once
#include <misc/ints.h>
#include <arch_constants.h>

// notify memory management of a new region of available RAM
void memory_ram_avail (u32 start, u32 end);

// allocate some memory
void* alloc (u32 size) __attribute__((assume_aligned(4 * sizeof(uptr))));

// free some memory
void free (void* ptr);

// allocate page aligned pages
void* alloc_pages (u32 count) __attribute__((assume_aligned(PAGE_SIZE)));

// free page aligned pages
void free_pages (void* ptr, u32 count);

// panic if out of memory?
extern int panic_on_oom;
