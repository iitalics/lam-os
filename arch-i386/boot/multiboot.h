#pragma once
#include <misc/ints.h>

// https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

struct multiboot_header {
    u32 flags;

    // [0] if flags & 0x1
    u32 mem_lower;
    u32 mem_upper;
    // [1] if flags & 0x2
    u32 boot_device;
    // [2] if flags & 0x4
    u32 cmdline;
    // [3] if flags & 0x8
    u32 mods_count;
    u32 mods_addr;
    // [4-5] if flags & 0x30
    u32 syms[4];
    // [6] if flags & 0x40
    u32 mmap_length;
    u32 mmap_addr;
    // [7] if flags & 0x80
    u32 drives_length;
    u32 drives_addr;
    // [8] if flags & 0x100
    u32 config_table;
    // [9] if flags & 0x200
    u32 boot_loader_name;
    // [10] if flags & 0x400
    u32 apm_table;
};
