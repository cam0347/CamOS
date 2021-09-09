#define HLTH 1

void sys_hlt();

void sys_hlt() {
    __asm__("cli");
    __asm__("hlt");
}