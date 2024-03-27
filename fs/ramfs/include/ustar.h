#pragma once
#include <include/types.h>

typedef struct {
    char name[100];
    uint64_t mode;
    uint64_t owner_id;
    uint64_t group_id;
    uint8_t size[12];
    uint8_t last_edit[12];
    uint64_t checksum;
    uint8_t type;
    
} ustar_descr_t;