/*
This module contains some basic interrupt handlers for debugging purposes and exception handling.
Once the kernel is loaded, these interrupts will be overwritten.

CPU interrupts:
0x00	* Division by zero
0x01	Single-step interrupt (see trap flag)
0x02	NMI
0x03	Breakpoint (which benefits from the shorter 0xCC encoding of INT 3)
0x04	* Overflow
0x05	Bound Range Exceeded
0x06	Invalid Opcode
0x07	Coprocessor not available
0x08	* Double Fault
0x09	Coprocessor Segment Overrun (386 or earlier only)
0x0A	Invalid Task State Segment
0x0B	Segment not present
0x0C	Stack Segment Fault
0x0D	General Protection Fault
0x0E	Page Fault
0x0F	reserved
0x10	x87 Floating Point Exception
0x11	Alignment Check
0x12	Machine Check
0x13	SIMD Floating-Point Exception
0x14	Virtualization Exception
0x15	Control Protection Exception (only available with CET)
*/

#include "int.h"
#include "types.h"

extern void *isr_ptr[];

struct idt_descriptor idt[256]; //bss
struct idtr idtr;               //bss

void idt_set_descriptor(uint8_t n, void *handler) {
    struct idt_descriptor *descr = &idt[n];
    descr -> offset_1 = (uint32_t) handler & 0x0000FFFF;
    descr -> offset_2 = (uint32_t) handler >> 16;
    descr -> selector = 0x08;
    descr -> type_attr = 0x8E;
    descr -> zero = 0x00;
}

void set_idt() {
    idtr.base = (uint32_t) idt;
    idtr.limit = (uint16_t)(sizeof(struct idt_descriptor) * 22 - 1);
    __asm__ volatile ("lidt %0" : : "memory"(idtr)); //load the new IDT
}

void init_idt() {
    for (int i = 0; i < 22; i++) {
        idt_set_descriptor((uint8_t) i, isr_ptr[i]);
    }
}