.set FLAGS,     1 | 2
.set MAGIC,     0x1badb002
.set CHECKSUM,  -(MAGIC + FLAGS)

  /* multiboot header */
  .section .multiboot
  .align 4
  .long MAGIC
  .long FLAGS
  .long CHECKSUM

  /* stack */
  .section .bss
  .align 16
stack_bot:  .skip 16*1024
stack_top:

  /* init */
  .section .text
  .global _start
  .type _start, @function

_start:
  mov $stack_top, %esp
  mov %ebx, %eax
  pushl %eax
/*  mov %ebx, multiboot_info_ptr */

  call kernel_init_0
  cli
1:  hlt
  jmp 1b


spin_loop:  jmp spin_loop

  .section .data
  .global multiboot_info_ptr
multiboot_info_ptr:
  .long 0
