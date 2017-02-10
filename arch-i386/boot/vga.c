#include "vga.h"
#include <misc/misc.h>

static u32 vga_row;
static u32 vga_col;
static u16 vga_mod;

void vga_init ()
{
    vga_row = vga_col = 0;
    vga_set_style(7, 0);
    vga_clear();
}

void vga_linebreak ()
{
    vga_col = 0;
    vga_row++;
}

void vga_print (char* str)
{
    for (char c; (c = *str); str++) {
        if (c == '\n') {
            vga_linebreak();
            continue;
        }

        VGA[vga_row * VGA_W + vga_col] = vga_mod | c;
        if (vga_col == VGA_W - 1)
            vga_linebreak();
        else
            vga_col++;
    }
}

void vga_clear ()
{
    for (u32 i = 0; i < (VGA_W * VGA_H); i++)
        VGA[i] = vga_mod | ' ';
}

void vga_set_style (u32 fg, u32 bg)
{
    vga_mod = ((fg & 0xf) << 8)
        | ((bg & 7) << 12);
}

void vga_move_to (u32 r, u32 c)
{
    vga_row = r;
    vga_col = c;
}
