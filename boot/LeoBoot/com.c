#include "com.h"
#include "types.h"
#include "tty.h"
#include "low_level.h"

void serial_print(const char *str) {
    uint32_t length = strlen(str);

    for (uint32_t i = 0; i < length; i++) {
        outb(0x3F8, *(uint8_t *)(str + i));
    }
}

//not working
void parallel_print(const char *str) {
    uint32_t length = strlen(str);

    for (uint32_t i = 0; i < length; i++) {
        outb(0x27A, *(uint8_t *)(str + i));
    }
}

static const char char_conv[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'};
char getc() {
    char input[2];
    input[1] = 0x00;

    while (1) {
        if ((inb(0x64) & 1) != 0) {
            input[0] = inb(0x60);

            if (input[0] >= 0 && input[0] < 84) {
                input[0] = char_conv[input[0]];
                print(input, VGA_COLOR_WHITE);
                return input[0];
            }
        }
    }
}