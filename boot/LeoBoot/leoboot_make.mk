include boot/LeoBoot/parser/parser_make.mk

bin/bootm.bin: boot/LeoBoot/*.c boot/LeoBoot/fs_drivers/*.* boot/LeoBoot/k_elf_loader/*.* boot/LeoBoot/parser/parser boot/LeoBoot/leoboot_make.mk
	i686-elf-gcc -ffreestanding -nostdlib -I boot/LeoBoot -Tboot/LeoBoot/ld_script -o bin/bootm.bin boot/LeoBoot/*.c boot/LeoBoot/fs_drivers/*.c boot/LeoBoot/k_elf_loader/*.c
