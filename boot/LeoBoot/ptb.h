#pragma once
#include "types.h"

enum pt_type {
    raw = 0,
    swap = 1,
    leofs = 2,
    ntfs = 3,
    fat16 = 4,
    fat32 = 5,
    zfs = 6,
    apfs = 7,
    nfs = 8
};

struct pt_entry {
    enum pt_type pt_type;
    uint32_t start_sector;
    uint32_t end_sector;
    uint8_t flags;
    uint8_t magic;
    uint8_t padding[2];
};

uint8_t read_pt(struct pt_entry table[]);
bool validate_part(struct pt_entry entry);
void print_entry_data(struct pt_entry entry);