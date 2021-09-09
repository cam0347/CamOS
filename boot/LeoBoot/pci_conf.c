#include "pci_conf.h"
#include "types.h"
#include "tty.h"
#include "low_level.h"
#include "mem.h"

void pci_explore() {
    /*uint16_t vid = pci_read_word(0, 0, 0, 0);
    uint16_t did = pci_read_word(0, 0, 0, 2);
    println_int(vid, VGA_COLOR_WHITE);
    println_int(did, VGA_COLOR_WHITE);*/
}

uint32_t make_address(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    return (uint32_t)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xfc) | ((uint32_t) 0x80000000));
}

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t _bus  = (uint32_t) bus;
    uint32_t _slot = (uint32_t) slot;
    uint32_t _func = (uint32_t) func;

    address = make_address(_bus, _slot, _func, offset);
 
    //write the address to CONFIG_ADDRESS
    outl(0xCF8, address);

    //returns the data from CONFIG_DATA
    return inw(0xCFC);
}

uint32_t pci_read_int(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t _bus  = (uint32_t) bus;
    uint32_t _slot = (uint32_t) slot;
    uint32_t _func = (uint32_t) func;

    address = address = make_address(_bus, _slot, _func, offset);

    //write the address to CONFIG_ADDRESS
    outl(0xCF8, address);

    //returns the data from CONFIG_DATA
    return inl(0xCFC);
}

void pci_read_header(uint8_t bus, uint8_t slot, uint8_t func, struct pci_device_header *header) {
    uint32_t data;

    for (int i = 0; i < 16; i++) {
        data = pci_read_int(bus, slot, func, 32 * i);
        memcpy(header + 32 * i, &data, 4);
    }
}

void pci_enum_bus() {
    uint16_t VID = 0, DID = 0;
    uint32_t class_code = 0;

    for (int bus = 0; bus < 256; bus++) {
        for (int dev = 0; dev < 32; dev++) {
            VID = pci_read_word(bus, dev, 0, 0);
            DID = pci_read_word(bus, dev, 0, 2);

            if (VID != 0xFFFF && DID != 0xFFFF) { //pci device found
                print("Device found: ", VGA_COLOR_WHITE);
                print_int(bus, VGA_COLOR_WHITE);
                print(" ", VGA_COLOR_WHITE);
                println_int(dev, VGA_COLOR_WHITE);
            }
        }
    }
}