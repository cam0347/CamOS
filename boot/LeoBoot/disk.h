#pragma once
#include "types.h"

#define PRIMARY_ATA_DATA 0x01F0
#define PRIMARY_ATA_ERROR 0x01F1
#define PRIMARY_ATA_SECTOR_COUNT 0x01F2
#define PRIMARY_ATA_LBA_LO  0x01F3
#define PRIMARY_ATA_LBA_MID 0x01F4
#define PRIMARY_ATA_LBA_HI  0x01F5
#define PRIMARY_ATA_DRVN_LBA_TOP 0x01F6
#define PRIMARY_ATA_COMMAND 0x01F7

static void ata_wait_data();
static void ata_wait_busy();
static bool ata_error();
static void switch_ata_error();
static void ata_cache_flush();
bool ata_read_sector(uint32_t lba, uint8_t count, void *buffer);
void ata_read(uint32_t start_block, uint16_t offset, void *buffer, uint32_t size);
void ata_read_addr(uint32_t address, void *buffer, uint32_t size);
bool ata_write_sector(uint32_t lba, void *data, uint32_t size);
void ata_write(uint32_t start_block, uint16_t offset, void *data, uint32_t size);