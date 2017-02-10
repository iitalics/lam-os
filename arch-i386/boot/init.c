#include <stdint.h>
#include <misc/misc.h>
extern void spin_loop ();

#define VGA_W     80
#define VGA_H     25
#define VGA   ((volatile uint16_t*) 0xb8000)

static inline uint16_t vga_c (char c, int fg, int bg)
{
    return c | (fg << 8) | (bg << 12);
}

static int vga_print_at (const char* str, int r, int c)
{
    int i;
    for (i = 0; str[i]; i++) {
        VGA[(c + i) + r * VGA_W] = vga_c(str[i], 11, 0);
    }
    return c + i;
}

void kernel_init_0 (char* multiboot_info_ptr)
{
    (void) multiboot_info_ptr;

    for (int y = 0; y < VGA_H; y++) {
        for (int x = 0; x < VGA_W; x++)
            VGA[x + y * VGA_W] = 0;
    }

    int r = 1;
    int c = 1;
    c = vga_print_at("Hello, ", r, c);
    char buf[ITOA_SIZE];
    itoa(buf, 12345, 10);
    c = vga_print_at(buf, r, c);
    c = vga_print_at("!", r, c);
}
