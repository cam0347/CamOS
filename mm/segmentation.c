#include <mm/include/segmentation.h>
#include <include/assert.h>
#include <include/low_level.h>
#include <include/mem.h>
#include <mm/include/paging.h>

gdt_t gdt[GDT_MAX_ENTRIES] __attribute__((aligned(sizeof(gdt_t))));
gdtr_t gdtr;
tss_t tss;

//loads basic segment descriptors and load the gdtr
bool setup_gdt() {
    assert_true(load_gdt(GDT_NULL_ENTRY, 0, 0, 0, 0)); //null descriptor
    assert_true(load_gdt(GDT_KERNEL_CS, 0, 0xFFFFF, 0x9A, 0xA)); //kernel mode code segment
    assert_true(load_gdt(GDT_KERNEL_DS, 0, 0xFFFFF, 0x92, 0xA)); //kernel mode data segment
    assert_true(load_gdt(GDT_USER_CS, 0, 0xFFFFF, 0xFA, 0xA)); //user mode code segment
    assert_true(load_gdt(GDT_USER_DS, 0, 0xFFFFF, 0xF2, 0xA)); //user mdoe data segment
    assert_true(load_gdt(GDT_TSS, (uint64_t) get_physical_address(&tss), sizeof(tss_t) - 1, 0x89, 0x0)); //TSS
    gdtr.base = (uint64_t) gdt;
    gdtr.size = 7 * sizeof(gdt_t) - 1; //6 descriptors but TSS's occupies twice the space
    disable_int();
    asm volatile("lgdt %0"::"m"(gdtr));
    enable_int();
    init_tss();
    return true;
}

//sets an entry of the gdt
bool load_gdt(uint32_t n, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    //this entry is extended (16 bytes instead of 8) if bit 4 of access byte is clear
    bool is_ext = (access >> 4 & 1) ? false : true;

    if ((!is_ext && n >= GDT_MAX_ENTRIES) || (is_ext && n >= GDT_MAX_ENTRIES - 1)) {
        return false;
    }

    gdt_t *entry = &gdt[n];
    entry->limit_0 = limit & 0xFFFF; //ignored
    entry->base_0 = base & 0xFFFF; //ignored
    entry->base_1 = base >> 16 & 0xFF; //ignored
    entry->access = access;
    entry->limit_1 = limit >> 16 & 0x0F; //ignored
    entry->flags = flags & 0x0F;
    entry->base_2 = base >> 24 & 0xFF; //ignored

    if (is_ext) {
        gdt_ext_t *ext_entry = (gdt_ext_t *) entry;
        ext_entry->base_3 = base >> 32 & 0xFFFFFFFF;
        ext_entry->reserved = 0;
    }

    return true;
}

void init_tss() {
    memclear(&tss, sizeof(tss_t));
    tss.esp0 = 0;
    tss.ss0 = GDT_KERNEL_DS * sizeof(gdt_t);
}

void load_tr() {
    uint16_t tss_selector = sizeof(gdt_t) * GDT_TSS;
    disable_int();
    asm volatile("mov %0, %%ax"::"r"(tss_selector));
    asm volatile("ltr %ax");
    enable_int();
}

//loads the new segment selectors into cpu's registers
void gdt_load_segments() {
    asm volatile(".intel_syntax");
    asm volatile("int 0x16");
    asm volatile(".att_syntax");
}