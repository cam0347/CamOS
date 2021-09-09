#include "low_level.h"
#include "types.h"
#include "abort.h"

static const uint32_t aux_stack_ebp = 0x2000;
static const uint32_t aux_stack_dim = 0x1B00; //6Kb
static uint32_t aux_stack_esp = 0x2000;
static bool aux_stack_usable = false;

uint8_t inb(uint16_t port) {
  uint8_t result;
  __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));

  return result;
}

void outb(uint16_t port, uint8_t data) {
  __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t inw(uint16_t port) {
  uint16_t result;
  __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));

  return result;
}

void outw(uint16_t port, uint16_t data) {
  __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

uint32_t inl(uint16_t port) {
  uint32_t result;
  __asm__("in %%dx, %%eax" : "=a" (result) : "d" (port));

  return result;
}

void outl(uint16_t port, uint32_t data) {
  __asm__("out %%eax, %%dx" : : "a" (data), "d" (port));
}

void insw(uint16_t port, void *data) {
  __asm__ volatile ("movl %0, %%edi" : : "r"((uint32_t) data));
  __asm__ volatile ("movw %0, %%dx" : : "r"(port));
  __asm__ volatile ("insw");
}

void aux_push(uint32_t data) {
  if (!aux_stack_usable) {
    abort("auxiliary stack accessed when disabled", null);
  }

  if (aux_stack_ebp - aux_stack_esp >= aux_stack_dim) {
    return;
  }

  //save the data to a register (free from the stack)
  __asm__("movl %0, %%edx" : : "r"(data));

  //save system base pointer and stack pointer
  __asm__("movl %ebp, %ebx");
  __asm__("movl %esp, %ecx");

  //switch to aux stack
  __asm__("movl %0, %%ebp" : : "r"(aux_stack_ebp));
  __asm__("movl %0, %%esp" : : "r"(aux_stack_esp));

  //push the data and save the new auxiliary stack pointer
  __asm__("pushl %edx");
  __asm__("movl %%esp, (%0)" : : "r"(&aux_stack_esp));

  //switch back to system stack
  __asm__("movl %ebx, %ebp");
  __asm__("movl %ecx, %esp");

  return;
}

uint32_t aux_pop() {
  if (!aux_stack_usable) {
    abort("auxiliary stack accessed when disabled", null);
  }
  
  if (aux_stack_esp >= aux_stack_ebp) {
    return 0;
  }

  uint32_t data;

  //save system base pointer and stack pointer
  __asm__("movl %ebp, %ebx");
  __asm__("movl %esp, %ecx");

  //switch to aux stack
  __asm__("movl %0, %%ebp" : : "r"(aux_stack_ebp));
  __asm__("movl %0, %%esp" : : "r"(aux_stack_esp));

  //pop the data and save the new auxiliary stack pointer
  __asm__("popl %edx");
  __asm__("movl %%esp, (%0)" : : "r"(&aux_stack_esp));

  //switch back to system stack
  __asm__("movl %ebx, %ebp");
  __asm__("movl %ecx, %esp");

  //transfer the data from the register to the memory location
  __asm__("movl %%edx, (%0)" : : "r"(&data));

  return data;
}

void aux_enable() {
  aux_stack_usable = true;
}

void aux_disable() {
  aux_stack_usable = false;
}

uint32_t put_eax(uint32_t data) {
  return data;
}

void sys_hlt() {
  __asm__("cli");
  __asm__("hlt");
}

void sleep(uint32_t s) {
  uint8_t pit_command = (0 << 6) | (2 << 4);
}