#pragma once
#include <include/types.h>
#define GDT_MAX_ENTRIES 8
#define GDT_NULL_ENTRY 0
#define GDT_KERNEL_CS 1
#define GDT_KERNEL_DS 2
#define GDT_USER_CS 3
#define GDT_USER_DS 4
#define GDT_TSS 5
#define UM_RING 3 //user mode ring number
#define KM_RING 0 //kernel mode ring number

typedef struct {
    /*uint16_t limit_0; //0-15
    uint16_t base_0; //0-15
    uint8_t base_1; //16-23
    uint8_t access;
    uint8_t limit_1:4; //16-19
    uint8_t flags:4;
    uint8_t base_2; //24-31
    uint32_t base_3; //32-63
    uint32_t reserved;*/
    uint16_t limit_0;
    uint16_t base_0;
    uint8_t base_1;
    uint8_t access;
    uint8_t limit_1:4;
    uint8_t flags:4;
    uint8_t base_2;
} __attribute__((packed)) gdt_t;

typedef struct {
	gdt_t std;
	uint32_t base_3; //upper 32 bits
	uint32_t reserved;
} __attribute__((packed)) gdt_ext_t;

typedef struct {
	uint32_t prev_tss;
	uint32_t esp0; //the stack pointer to load when changing to kernel mode
	uint32_t ss0;  //the stack segment to load when changing to kernel mode
	//everything below here is unused
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed)) tss_t;


typedef struct {
    uint16_t size;
    uint64_t base;
} __attribute((packed)) gdtr_t;

bool setup_gdt();
bool load_gdt(uint32_t, uint64_t, uint32_t, uint8_t, uint8_t);
void gdt_load_segments();
void init_tss();
void load_tr();