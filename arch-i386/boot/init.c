#include <stdint.h>
#include "vga.h"
#include <misc/misc.h>

void kernel_init_0 (char* multiboot_info_ptr)
{
    (void) multiboot_info_ptr;

    vga_init();

    char buffer[32];
    set_writef_output_buffer(buffer, sizeof buffer);
    writef("The answer is {d}, you are {}!",
           42, "correct");
    vga_print(buffer);
}
