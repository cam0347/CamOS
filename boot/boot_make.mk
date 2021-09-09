include boot/LeoBoot/leoboot_make.mk

bin/boot1.bin: boot/boot1.asm boot/include/* boot/boot_make.mk
	nasm -f bin -I boot -o bin/boot1.bin boot/boot1.asm

bin/boot2.bin: boot/boot2.asm boot/include/* boot/boot_make.mk
	nasm -f bin -I boot -o bin/boot2.bin boot/boot2.asm
