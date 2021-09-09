#pragma once
#include "types.h"

//64 byte length
struct pci_device_header {
    uint16_t vendor;
    uint16_t id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint32_t class_code: 24;
    uint8_t cache_line;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t BIST;
    uint32_t bars[6];
    uint32_t cardbus_cis_pointer;
    uint16_t subsystem_vendor;
    uint16_t subsystem_id;
    uint32_t expansion_rom_address;
    uint8_t capabilities_pointer;
    uint32_t reserved1: 24;
    uint32_t reserved2;
    uint8_t int_line;
    uint8_t int_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} __attribute__((packed));

//prototypes
uint32_t make_address(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_read_int(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_read_header(uint8_t bus, uint8_t slot, uint8_t func, struct pci_device_header *header);
void pci_enum_bus();