#pragma once

#include "types.h"

uint32_t memcpy(void *dest, void *src, uint32_t size);
uint32_t memmove(void *dest, void *src, uint32_t size);
void memclear(void *ptr, uint32_t size);
void reverse_endianess(void *ptr, uint32_t length);