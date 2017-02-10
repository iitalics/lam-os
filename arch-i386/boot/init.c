#include <stdint.h>
extern void spin_loop ();

#define VGA_W     80
#define VGA_H     25
#define VGA   ((volatile uint16_t*) 0xb8000)

static inline uint16_t vga_c (char c, int fg, int bg)
{
    return c | (fg << 8) | (bg << 12);
}

void kernel_init_0 (char* multiboot_info_ptr)
{
    (void) multiboot_info_ptr;

    for (int y = 0; y < VGA_H; y++) {
        for (int x = 0; x < VGA_W; x++)
            VGA[x + y * VGA_W] = 0;
    }

    const char* const hello = "Hello, world";
    for (int i = 0; hello[i]; i++) {
        VGA[(1 + i) + 1 * VGA_W] = vga_c(hello[i], 11, 0);
    }
}
