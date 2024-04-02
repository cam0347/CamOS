# CamOS kernel
## What's CamOS
CamOS is a 64 bit toy/hobby operating system, i'm doing this just because i wanted to know more on how an operating system works, and because.. why not.
This is just a command-line OS, nothing special...

## Architecture
CamOS has its own UEFI bootloader LeoBoot, which sets up a minimal environment for the kernel to keep initializing the machine.
Once the bootloader jumps to the kernel (which is statically loaded at 0xFFFFFFFFFFEFF000) the kernel bootstraps itself
to an environment suitable to run user code.

## Dependencies
gcc and ld, not bad is it? :)

## How to compile
INPUT_FILES = $(shell find src -not -name "isr.c" -name "*.c")
gcc -c -ffreestanding -nostdlib -O0 -m64 -no-pie -Isrc $(INPUT_FILES)
gcc -c -ffreestanding -nostdlib -O0 -m64 -no-pie -mgeneral-regs-only -Isrc src/int/*.c
ld -T linker.ld *.o -o leokernel.elf

the linker script (linker.ld) is not present here, i'll upload it later on