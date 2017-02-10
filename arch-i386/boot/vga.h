#include <stdint.h>

#define VGA_W     80
#define VGA_H     25
#define VGA   ((volatile uint16_t*) 0xb8000)

void vga_init ();
void vga_move_to (int r, int c);
void vga_print (char* str);
void vga_set_style (int fg, int bg);
void vga_clear ();
