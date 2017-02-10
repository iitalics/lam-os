#include <stdint.h>
#include "vga.h"


void kernel_init_0 (char* multiboot_info_ptr)
{
    (void) multiboot_info_ptr;

    vga_init();
    vga_print("Hello, world!");
    vga_move_to(10, 30);
    vga_print("Hello, world!");
}
