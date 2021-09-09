#include "disk.h"
#include "types.h"
#include "low_level.h"
#include "tty.h"
#include "mem.h"
#include "math.h"

static void ata_wait_data() {
    while(!(inb(PRIMARY_ATA_COMMAND) >> 3)) {}
}

static void ata_wait_busy() {
    uint8_t in;
    while((in = inb(PRIMARY_ATA_COMMAND)) && in >> 7 && !(in >> 6)) {}
}

static bool ata_error() {
    return inb(PRIMARY_ATA_COMMAND) & 1;
}

static void switch_ata_error() {
    switch(inb(PRIMARY_ATA_ERROR)) {
            case 1:
                error("address mark not found");
                break;
            
            case 2:
                error("track 0 not found");
                break;

            case 4:
                error("command aborted");
                break;

            case 8:
                error("media change requested");
                break;

            case 16:
                error("ID mark not found");
                break;

            case 32:
                error("media changed");
                break;

            case 64:
                error("uncorrectable data error");
                break;

            case 128:
                error("bad block");
                break;
            
            default:
                error("multiple or unknown error");
                break;
        }
}

static void ata_cache_flush() {
    outb(PRIMARY_ATA_COMMAND, 0xE7); //cache flush command
    ata_wait_busy();
}

bool ata_read_sector(uint32_t lba, uint8_t count, void *buffer) {
    ata_wait_busy();
    outb(PRIMARY_ATA_SECTOR_COUNT, count);                       //number of sectors
    outb(PRIMARY_ATA_LBA_LO, (lba >> 0)  & 0xFF);                //lowest byte of sector
    outb(PRIMARY_ATA_LBA_MID, (lba >> 8)  & 0xFF);               //middle byte of sector
    outb(PRIMARY_ATA_LBA_HI, (lba >> 16) & 0xFF);                //highest byte of sector
    outb(PRIMARY_ATA_DRVN_LBA_TOP, 0xE0 | ((lba >> 24) & 0x0F)); //highest 4 bits of sector and drive number (0x00 or 0x10)
    outb(PRIMARY_ATA_COMMAND, 0x20);                             //read command
    ata_wait_data();

    for (int i = 0; i < count; i++) {
        ata_wait_data();

        for (int j = 0; j < 256; j++) {
            *(uint16_t *)(buffer + i * 512 + j * 2) = inw(PRIMARY_ATA_DATA);
        }

        if (ata_error()) {
            switch_ata_error();
            return false;
        }

        ata_cache_flush();
    }

    return true;
}

//read from the disk from the specified sector lba (starting at 0)
void ata_read(uint32_t start_block, uint16_t offset, void *buffer, uint32_t size) {
    if (offset > 511) {
        offset %= 512;
    }

    uint32_t tot_offset = 512 * start_block + offset + size;
    uint32_t start_lba = start_block;
    uint32_t end_lba = tot_offset / 512 + 1;
    uint32_t count = end_lba - start_lba + 1;
    uint8_t tmp[512 * count];

    for (uint32_t i = 0; i < count; i++) {
        ata_read_sector(start_lba + i, 1, &tmp[512 * i]);
    }
    
    memcpy(buffer, (void *) &tmp[offset], size);
}

void ata_read_addr(uint32_t address, void *buffer, uint32_t size) {
    uint32_t sector = address / 512;
    uint16_t offset = address % 512;
    ata_read(sector, offset, buffer, size);
}

bool ata_write_sector(uint32_t lba, void *data, uint32_t size) {
    uint8_t count = (uint8_t) ceil((double) size / (double) 512);

    ata_wait_busy();
    outb(PRIMARY_ATA_SECTOR_COUNT, count);                       //number of sectors
    outb(PRIMARY_ATA_LBA_LO, (lba >> 0)  & 0xFF);                //lowest byte of sector
    outb(PRIMARY_ATA_LBA_MID, (lba >> 8)  & 0xFF);               //middle byte of sector
    outb(PRIMARY_ATA_LBA_HI, (lba >> 16) & 0xFF);                //highest byte of sector
    outb(PRIMARY_ATA_DRVN_LBA_TOP, 0xE0 | ((lba >> 24) & 0x0F)); //highest 4 bits of sector and drive number (0x00 or 0x10)
    outb(PRIMARY_ATA_COMMAND, 0x30);                             //write command
    ata_wait_data();

    for (uint32_t i = 0; i < size; i++) {
        outb(PRIMARY_ATA_DATA, *(uint8_t *)(data + i));
    }

    if (ata_error()) {
        switch_ata_error();
        return false;
    } else {
        return true;
    }
}

//non funziona per adesso
void ata_write(uint32_t start_block, uint16_t offset, void *data, uint32_t size) {
    
}