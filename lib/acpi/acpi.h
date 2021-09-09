#ifndef TYPESH
    #include "../types.h"
#endif

struct rsdp_descriptor_legacy {
    char signature[8];
    uint8_t shecksum;
    char oem[6];
    uint8_t revision;
    uint32_t rsdt_dddress;
};

struct rsdp_descriptor {
    struct rsdp_descriptor_legacy legacy;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t ext_checksum;
    uint8_t reserved[3];
};