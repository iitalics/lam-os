#include <stdint.h>
#include <misc/misc.h>
#include "vga.h"
#include "multiboot.h"
#include <kernel/kernel.h>
#include <memory/memory.h>

// render integer in binary; for {?} format
static void write_binary16 (uptr x) {
    uptr k = 1 << 15;
    char buf[17];
    for (int i = 0; i < 16; i++) {
        buf[i] = (x & k) ? '1' : '0';
        k /= 2;
    }
    buf[16] = '\0';
    writef_output(buf);
}

// render integer as megabytes; for {?} format
static void write_mb (uptr x) {
    const int M = 1024 * 1024;
    writef("{du}.{du}M",
           (u32) (x / M),
           (u32) (x * 10 / M % 10));
}


void kernel_mmaps (u32 addr, u32 length)
{
    u32 pos = 0;
    for (int i = 1; pos < length; i++) {
        struct multiboot_mmap* mmap = (void*) (addr + pos);
        pos += mmap->size + sizeof(u32);

        int avail = mmap->type == 1
            // 32 bit address only
            && mmap->base_addr_hi == 0
            && mmap->length_hi == 0
            // don't register the zero page
            && mmap->base_addr_lo != 0;

        if (!avail)
            continue;

        memory_ram_avail(mmap->base_addr_lo,
                         mmap->base_addr_lo + mmap->length_lo);
    }
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

    if (flags & 0x200) {
        writef("  Boot loader:   \"{}\"\n",
               (const char*) mb_header->boot_loader_name);
    }
    if (flags & 0x4) {
        writef("  Command line:  \"{}\"\n",
               (const char*) mb_header->cmdline);
    }
    if (flags & 0x1) {
        // 0x27f -> 0x1fb80
        writef("  Lower memory:  {?}\n"
               "  Upper memory:  {?}\n",
               write_mb, (uptr) mb_header->mem_lower * 1024,
               write_mb, (uptr) mb_header->mem_upper * 1024);
    }

    /* if (flags & 0x2) { */
    /*     writef("  boot device:   {xu}\n", */
    /*            mb_header->boot_device); */
    /* } */
    /* if (flags & 0x8) { */
    /*     writef("  mods count:    {du}\n" */
    /*            "       addr:     {p}\n", */
    /*            mb_header->mods_count, */
    /*            (uptr) mb_header->mods_addr); */
    /* } */

    if (flags & 0x40) {
        kernel_mmaps(mb_header->mmap_addr,
                     mb_header->mmap_length);
    }

    /* if (flags & 0x80) { */
    /*     writef("  drives length: {du}\n" */
    /*            "         addr:   {p}\n", */
    /*            mb_header->drives_length, */
    /*            (uptr) mb_header->drives_addr); */
    /* } */

    char* strings[4] = {0};
    panic_on_oom = 1;
    vga_set_style(2 | 8, 0);

    for (i32 i = 0; i < 4; i++) {
        strings[i] = k_alloc(15);
        writef("strings[{d}] = {p}\n",
               i, strings[i]);
    }
    writef("----------\n");
    for (i32 i = 0; i < 4; i++) {
        k_free(strings[i]);
    }
    for (i32 i = 0; i < 4; i++) {
        strings[i] = k_alloc(15);
        writef("strings[{d}] = {p}\n",
               i, strings[i]);
    }
}
