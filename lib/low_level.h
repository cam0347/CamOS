/*
ZAVATTARO CAMILLO
27/04/2021

Low level routines
*/

#define LOWLEVELH 1

#ifndef TYPESH
  #include "types.h"
#endif

//read a byte from an I/O port
uint8_t inb(uint16_t port) {
  uint8_t result;
  __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));

  return result;
}

//write a byte to an I/O port
void outb(uint16_t port, uint8_t data) {
  __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

//read a word from an I/O port
uint16_t inw(uint16_t port) {
  uint16_t result;
  __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));

  return result;
}

//write a word to an I/O port
void outw(uint16_t port, uint16_t data) {
  __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

//read 4 bytes from an I/O port
uint32_t inl(uint16_t port) {
  uint32_t result;
  __asm__("in %%dx, %%eax" : "=a" (result) : "d" (port));

  return result;
}

//write 4 bytes to an I/O port
void outl(uint16_t port, uint32_t data) {
  __asm__("out %%eax, %%dx" : : "a" (data), "d" (port));
}

