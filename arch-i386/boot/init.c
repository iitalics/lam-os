#include <stdint.h>
#include "vga.h"
#include <misc/misc.h>
#include <limits.h>

void kernel_init_0 (char* multiboot_info_ptr)
{
    (void) multiboot_info_ptr;

    vga_init();
    writef_output = vga_print;

    for (u32 i = 0; i < 100; i++) {
        writef("{d}\n", i);
    }
}
