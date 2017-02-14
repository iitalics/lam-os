#include <boot/vga.h>
#include <kernel/kernel.h>
#include <misc/misc.h>

char kp_buf[VGA_W + 1] = {0};
char* kernel_panic_str = kp_buf;
u32 kernel_panic_str_cap = sizeof kp_buf;

void kernel_panic ()
{
    vga_set_style(0, 7);
    vga_clear();
    vga_move_to(5, (VGA_W - 18) / 2);
    vga_print("== KERNEL PANIC ==");
    vga_move_to(7, (VGA_W - str_len(kernel_panic_str)) / 2);
    vga_print(kernel_panic_str);

    __asm(
        "1: cli\n"
        "   hlt\n"
        "   jmp 1b\n"
        );
}
