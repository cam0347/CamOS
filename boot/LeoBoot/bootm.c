/*
CamOS boot manager
Loaded at 0x8610 (16 bytes after bootloader stage 2)
Located on the disk from sector 6 to 261

disk sectors:
code: 6 ~ 133 (128 sectors, 64Kb)
data: 134 ~ 197: (64 sectors, 32Kb)
rodata: 198 ~ 261: (64 sectors, 32Kb)

memory segments:


Warning: 32 bit code, make sure we're in protected mode

This program has to:
- loads temporary interrupt handlers
- read the partition table to find bootable partitions
- explore the selected partition's file system to load the kernel
- eventually pass some startup parameters to the kernel


TO DO:
- implement relocation function to the kernel loader
- write a sleep() routine
- implement LeoFS driver
*/

#include "bootm.h"
#include "types.h"
#include "tty.h"
#include "ptb.h"
#include "low_level.h"
#include "int.h"
#include "ptb.h"
#include "string.h"

void boot() {
    clear_vga();
    enable_cursor();
    println("LeoBoot 1.0", VGA_COLOR_GREEN);

    print("Cleaning memory... ", VGA_COLOR_WHITE);
    clean_stack_mem();
    println("done", VGA_COLOR_GREEN);

    print("Setting up stack... ", VGA_COLOR_WHITE);
    setup_aux_stack();
    println("done", VGA_COLOR_GREEN);

    print("Setting up interrupts... ", VGA_COLOR_WHITE);
    init_idt();
    set_idt();
    println("done", VGA_COLOR_GREEN);

    print("Parsing partition table... ", VGA_COLOR_WHITE);
    struct pt_entry entry = parse_pt(); //get the partition table entry of the selected bootable partition
    println("done", VGA_COLOR_GREEN);

    print("Loading kernel image... ", VGA_COLOR_WHITE);
    //determine the type of partition in order to load the kernel correctly
    switch(entry.pt_type) {
        case leofs:
            break;

        default:
            error("Unknown partition type, halting...");
            sys_hlt();
            break;
    }

    sys_hlt();
}

void clean_stack_mem() {
    //this routine erase all the previous code loaded in memory (boot loader st. 1 and 2)
    //WARNING: can cause some troubles when too much functions are called before this routine (256 bytes bound for previous stack frames)
    for (uint32_t i = 0x8500; i >= 0x4600; i -= 4) {
        *(int *) i = 0x00000000;
    }
}

void setup_aux_stack() {
    for (uint32_t i = 0x0800; i >= 0x0500; i -= 4) {
        *(int *) i = 0x00000000;
    }

    aux_enable();
}

struct pt_entry parse_pt() {
    struct pt_entry table[32]; //max number of entries
    uint32_t leofs_part[32], leofs_count = 0;
    uint8_t rows = read_pt(table);

    if (rows == 0) {
        error("0 partitions found, rebooting...");
        sys_hlt();
    } else {
        for (uint8_t i = 0; i < rows; i++) {
            switch(table[i].pt_type) {
                case leofs:
                    leofs_part[leofs_count] = (uint32_t) i;
                    leofs_count++;
                    break;

                default:
                    break;
            }
        }
    }

    if (leofs_count == 0) {
        error("0 partitions with supported file system, halting...\n(it may be a temporaty problem, try to reboot the machine)");
        sys_hlt();
    } else if (leofs_count == 1) {
        return table[leofs_part[0]];
    } else {
        int choice;
        println("", VGA_COLOR_WHITE);

        for (int i = 0; i < leofs_count; i++) {
            if (validate_part(table[leofs_part[i]])) {
                print_int(i, VGA_COLOR_WHITE);
                print(": ", VGA_COLOR_WHITE);
                print_entry_data(table[leofs_part[i]]);
            }
        }

        print("Which one would you like to use?: ", VGA_COLOR_WHITE);
        choice = scan_int();
        
        if (choice < 0 || choice > leofs_count - 1) {
            error("Invalid choice, halting...");
            sys_hlt();
        } else {
            return table[leofs_part[choice]];
        }
    }
}