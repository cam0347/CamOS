#pragma once

#include "types.h"

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);
void insw(uint16_t port, void *data);
void aux_push(uint32_t data);
uint32_t aux_pop();
void aux_enable();
void aux_disable();
uint32_t put_eax(uint32_t data);
void sys_hlt();
void sleep(uint32_t s);