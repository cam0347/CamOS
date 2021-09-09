#include "mem.h"
#include "types.h"

uint32_t memcpy(void *dest, void *src, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        *(char *)(dest + i) = *(char *)(src + i);
    }

    return size;
}

uint32_t memmove(void *dest, void *src, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        *(char *)(dest + i) = *(char *)(src + i);
        *(char *)(src + i) = 0x00;
    }

    return size;
}

void memclear(void *ptr, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        *(char *)(ptr + i) = 0x00;
    }
}

void reverse_endianess(void *ptr, uint32_t length) {
    if (length <= 1) {
        return;
    }

    register char tmp;

    for (int i = 0; i < length / 2; i++) {
        tmp = *(char *)(ptr + i);
        *(char *)(ptr + i) = *(char *)(ptr + (length - 1) - i);
        *(char *)(ptr + (length - 1) - i) = tmp;
    }
}