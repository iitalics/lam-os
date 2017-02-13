#include <stdint.h>
#include <misc/misc.h>
#include "vga.h"
#include "multiboot.h"

void write_binary16 (uptr x) {
    uptr k = 1 << 15;
    char buf[17];
    for (int i = 0; i < 16; i++) {
        buf[i] = (x & k) ? '1' : '0';
        k /= 2;
    }
    buf[16] = '\0';
    writef_output(buf);
}

void kernel_init_0 (const struct multiboot_header* mb_header)
{
    vga_init();
    writef_output = vga_print;

    u32 flags = mb_header->flags;
    vga_set_style(15, 0);
    writef("== multiboot header ==\n");
    vga_set_style(7, 0);
    writef("  flags: {?} ({x})\n",
           write_binary16, (uptr) flags,
           flags);

    if (flags & 0x1) {
        // 0x27f -> 0x1fb80
        writef("  mem:           {p}\n"
               "              => {p}\n",
               (void*) mb_header->mem_lower,
               (void*) mb_header->mem_upper);
    }
    if (flags & 0x2) {
        writef("  boot device:   {xu}\n",
               mb_header->boot_device);
    }
    if (flags & 0x4) {
        writef("  cmdline:       {xu}\n",
               mb_header->cmdline);
    }
    if (flags & 0x8) {
        writef("  mods count:    {du}\n"
               "       addr:     {p}\n",
               mb_header->mods_count,
               (void*) mb_header->mods_addr);
    }
    if (flags & 0x40) {
        writef("  mmap length:   {du}\n"
               "       addr:     {p}\n",
               mb_header->mmap_length,
               (void*) mb_header->mmap_addr);
    }
    if (flags & 0x80) {
        writef("  drives length: {du}\n"
               "         addr:   {p}\n",
               mb_header->drives_length,
               (void*) mb_header->drives_addr);
    }
    if (flags & 0x200) {
        writef("  boot loader:   {xu}\n",
               mb_header->boot_loader_name);
    }
}
