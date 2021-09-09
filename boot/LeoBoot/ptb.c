#include "ptb.h"
#include "types.h"
#include "tty.h"
#include "disk.h"
#include "mem.h"

//read partition table
uint8_t read_pt(struct pt_entry table[]) {
    uint8_t i = 0;
    bool end = 0;
    char ptb[512];
    struct pt_entry ent;

    ata_read_sector(1, 1, ptb);

    for (i = 0; i < 32; i++) {
        memcpy(&ent, &ptb[16 * i], 16);

        if (ent.magic != 0x00) {
            reverse_endianess(&ent.start_sector, 4);
            reverse_endianess(&ent.end_sector, 4);
            reverse_endianess(&ent.pt_type, 4);
            memcpy(&table[i], &ent, 16);
        } else {
            break;
        }
    }

    return i;
}

bool validate_part(struct pt_entry entry) {
    return (bool)(((entry.flags >> 7) & 1) && ((entry.flags >> 0) & 1) && !((entry.flags >> 3) & 1));
}

void print_entry_data(struct pt_entry entry) {
    print_hex(entry.start_sector, VGA_COLOR_WHITE); print(" ", VGA_COLOR_WHITE);
    print_hex(entry.end_sector, VGA_COLOR_WHITE);   print(" (", VGA_COLOR_WHITE);
    print_int((uint32_t)((entry.end_sector - entry.start_sector) * 512 / 1024), VGA_COLOR_WHITE);
    print(" Kbytes) ", VGA_COLOR_WHITE);

    switch(entry.pt_type) {
        case raw:
            print("raw ", VGA_COLOR_WHITE);
            break;

        case swap:
            print("swap ", VGA_COLOR_WHITE);
            break;

        case leofs:
            print("leofs ", VGA_COLOR_WHITE);
            break;

        case ntfs:
            print("ntfs ", VGA_COLOR_WHITE);
            break;

        case fat16:
            print("fat16 ", VGA_COLOR_WHITE);
            break;

        case fat32:
            print("fat32 ", VGA_COLOR_WHITE);
            break;

        case zfs:
            print("zfs ", VGA_COLOR_WHITE);
            break;

        case apfs:
            print("apfs ", VGA_COLOR_WHITE);
            break;

        case nfs:
            print("nfs ", VGA_COLOR_WHITE);
            break;

        default:
            print("unknown ", VGA_COLOR_WHITE);
    }

    if ((entry.flags >> 6) & 1) {
        print("[SWAP] ", VGA_COLOR_WHITE);
    }

    if ((entry.flags >> 4) & 1) {
        print("[DAMAGED] ", VGA_COLOR_RED);
    }

    if ((entry.flags >> 2) & 1) {
        print("[LOCKED] ", VGA_COLOR_LIGHT_BROWN);
    }

    print("\n", VGA_COLOR_WHITE);
}