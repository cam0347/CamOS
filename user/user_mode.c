#include <include/types.h>
#include <user/include/user_mode.h>
#include <mm/include/segmentation.h>

bool init_um(void) {

    /*
    - Add two new GDT entries (at least) configured for ring 3.
    - Set up a barebones TSS with an ESP0 stack.
    - Set up an IDT entry for ring 3 system call interrupts (optional).
    */

    //user data and user code segments are already loaded in the gdt
    load_tr(); //loads task register
    return true;
}

/*
this function is for test purposes only.
the switch between user and kernel mode will be handled by another module
*/
void enter_um() {
    uint16_t user_code = (GDT_USER_CS * sizeof(gdt_t)) | UM_RING;
    uint16_t user_data = (GDT_USER_DS * sizeof(gdt_t)) | UM_RING;
    asm volatile("mov %0, %%ax"::"r"(user_data));
    asm volatile("mov %ax, %ds");
    asm volatile("mov %ax, %es");
    asm volatile("mov %ax, %fs");
    asm volatile("mov %ax, %gs");
    asm volatile("xor %edx, %edx");
    asm volatile("mov $0x8, %eax");
    asm volatile("mov $0x174, %ecx");
    asm volatile("wrmsr");
    asm volatile("mov %0, %%rdx"::"r"(um_test));
    asm volatile("mov %esp, %ecx");
    //asm volatile("hlt");
    asm volatile("sysexit");
}

/*
this function is for test purposes only.
this function is used to test user mode behaviour, it simulates a program
*/
void um_test() {

}