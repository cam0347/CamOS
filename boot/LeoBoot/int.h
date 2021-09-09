#pragma once
#include "types.h"

//8 byte
struct idt_descriptor {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
} __attribute__((packed));

struct idtr {
    uint16_t limit; //size of the ldt table - 1
    uint32_t base;  //the base address of the table
} __attribute__((packed));

void idt_set_descriptor(uint8_t n, void *handler);
void set_idt();
void init_idt();
extern void gen_isr();