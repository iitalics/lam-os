export cc-base=gcc -m32
export ld-base=gcc -m32 -ffreestanding
export as-base=gcc -m32

export cflags=-Wall -O2
export asflags=
export ldflags=-Wl,--build-id=none
