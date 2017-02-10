#include <misc/ints.h>

#define VGA_W     80
#define VGA_H     25
#define VGA   ((volatile u16*) 0xb8000)

void vga_init ();
void vga_move_to (u32 r, u32 c);
void vga_print (char* str);
void vga_set_style (u32 fg, u32 bg);
void vga_linebreak ();
void vga_clear ();
