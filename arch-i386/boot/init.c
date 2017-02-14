#include <stdint.h>
#include <misc/misc.h>
#include "vga.h"
#include "multiboot.h"
#include <kernel/kernel.h>

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



static void memory_add_avail_ram(char* addr, u32 len)
{
    writef("    Found RAM: {?} @ {p}\n",
           write_mb, len,
           addr);
}

void kernel_mmaps (u32 addr, u32 length)
{
    u32 pos = 0;
    for (int i = 1; pos < length; i++) {
        struct multiboot_mmap* mmap = (void*) (addr + pos);
        pos += mmap->size + sizeof(u32);

        if (mmap->type == 1
            // 32 bit address only
            && mmap->base_addr_hi == 0
            && mmap->length_hi == 0
            // don't register the zero page
            && mmap->base_addr_lo != 0) {
            memory_add_avail_ram((void*)mmap->base_addr_lo, mmap->length_lo);
        }
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

    kernel_panicf("init routine ended early.");
}
