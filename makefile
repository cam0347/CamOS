include boot/boot_make.mk
include kernel/kernel_make.mk

disk.iso: bin/boot1.bin bin/boot2.bin bin/bootm.bin bin/kernel.bin
	./etc/disk_build.sh
	./boot/LeoBoot/parser/parser
	./partition_tool/partition_tools master
	rm bin/bootm.bin
	mv bin/kernel.bin bin/bootm.bin
	clear
	boot/LeoBoot/parser/parser

all: disk.iso
