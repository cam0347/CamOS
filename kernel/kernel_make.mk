bin/kernel.bin: kernel/main.c
	i686-elf-gcc -ffreestanding -nostdlib -I . -r -Tkernel/ld_script.ld -o bin/kernel.bin kernel/main.c
