#include "isr.h"
#include "tty.h"
#include "low_level.h"

void *isr_ptr[22] = {
    isr_0,
    isr_1,
    isr_2,
    isr_3,
    isr_4,
    isr_5,
    isr_6,
    isr_7,
    isr_8,
    isr_9,
    isr_10,
    isr_11,
    isr_12,
    isr_13,
    isr_14,
    isr_15,
    isr_16,
    isr_17,
    isr_18,
    isr_19,
    isr_20,
    isr_21
};

void gen_isr() {
    __asm__(".intel_syntax noprefix");
    __asm__("mov eax, 0xC1A0");
    __asm__("hlt");
    __asm__(".att_syntax noprefix");
    __asm__("iret");
}

void isr_0() {
    int_log("division by zero, be careful", 0);
    sys_hlt();
}

void isr_1() {
    int_log("single step interrupt", 1);
    sys_hlt();
}

void isr_2() {
    int_log("non-maskable interrupt", 2);
    sys_hlt();
}

void isr_3() {
    int_log("breakpoint", 3);
    sys_hlt();
}

void isr_4() {
    int_log("oops.. overflow", 4);
    sys_hlt();
}

void isr_5() {
    int_log("oops.. bound range exceeded", 5);
    sys_hlt();
}

void isr_6() {
    int_log("mhh.. i don't know what to do, invalid opcode", 6);
    sys_hlt();
}

void isr_7() {
    int_log("coprocessor took a break, he's not available", 7);
    sys_hlt();
}

void isr_8() {
    int_log("wow.. double fault, what have you done??", 8);
    sys_hlt();
}

void isr_9() {
    int_log("coprocessor segment overrun", 9);
    sys_hlt();
}

void isr_10() {
    int_log("invalid TSS", 10);
    sys_hlt();
}

void isr_11() {
    int_log("404 segment not present", 11);
    sys_hlt();
}

void isr_12() {
    int_log("the stack made a fault.. or its segment", 12);
    sys_hlt();
}

void isr_13() {
    int_log("general protection fault, be careful", 13);
    sys_hlt();
}

void isr_14() {
    int_log("404 page not found", 14);
    sys_hlt();
}

void isr_15() {
    int_log("sorry.. reserved interrupt", 15);
    sys_hlt();
}

void isr_16() {
    int_log("floating point exception", 16);
    sys_hlt();
}

void isr_17() {
    int_log("alignment check.. i don't even know what it means", 17);
    sys_hlt();
}

void isr_18() {
    int_log("machine exception.. me neither", 18);
    sys_hlt();
}

void isr_19() {
    int_log("oh gosh.. SIMD floating point exception", 19);
    sys_hlt();
}

void isr_20() {
    int_log("virtualization exception", 20);
    sys_hlt();
}

void isr_21() {
    int_log("control protection exception", 21);
    sys_hlt();
}