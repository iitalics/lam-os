#include "vga.h"
#include <misc/misc.h>

static int vga_row;
static int vga_col;
static uint16_t vga_mod;

void vga_init ()
{
    vga_row = vga_col = 0;
    vga_set_style(7, 0);
    vga_clear();
}

static void vga_linebreak ()
{
    vga_col = 0;
    vga_row++;
}

void vga_print (char* str)
{
    for (char c; (c = *str); str++) {
        if (c == '\n')
            vga_linebreak();
        else
            VGA[vga_row * VGA_W + vga_col] = vga_mod | c;
        if (vga_col == VGA_W - 1)
            vga_linebreak();
        else
            vga_col++;
    }
}

void vga_clear ()
{
    for (int i = 0; i < (VGA_W * VGA_H); i++)
        VGA[i] = vga_mod | ' ';
}

void vga_set_style (int fg, int bg)
{
    vga_mod = ((fg & 0xf) << 8)
        | ((bg & 7) << 12);
}

void vga_move_to (int r, int c)
{
    vga_row = r;
    vga_col = c;
}
