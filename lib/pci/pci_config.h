/*
These functions are used to configure pci devices
*/

#define PCI_CONFIGH 1

#ifndef TYPESH
    #include "../types.h"
#endif

#ifndef LOWLEVELH
    #include "../low_level.h"
#endif

#ifndef VIDEOH
    #include "../video/video.h"
#endif

#ifndef VGACOLORSH
    #include "../video/vga_colors.h"
#endif

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

uint16_t pci_conf_read_word(uint32_t bus, uint32_t dev, uint32_t func, uint8_t offset);
int pci_explore();

struct pci_device_header {
    uint16_t vendor;
    uint16_t id;
    uint16_t command;
    uint16_t status;
};

uint16_t pci_conf_read_word(uint32_t bus, uint32_t dev, uint32_t func, uint8_t offset) {
    uint32_t address;
    uint16_t tmp;

    //make the address
    address = (uint32_t)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000));

    outl(CONFIG_ADDRESS, address); //send the request
    tmp = (uint16_t)((inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF); //read the answer

    return tmp;
}

int pci_explore() {
    for (int i = 0; i < 10; i++) {
        print_int((int) pci_conf_read_word(0, (uint32_t) i, 0, 0), VGA_COLOR_RED);
    }

    return 1;
}