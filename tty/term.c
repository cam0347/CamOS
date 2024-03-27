#include <include/types.h>
#include <tty/include/term.h>
#include <tty/include/tty.h>
#include <io/include/keyboard.h>
#include <io/include/files.h>
#include <mm/include/kmalloc.h>
#include <include/string.h>
#include <include/assert.h>
#include <include/mem.h>
#include <tty/include/def_colors.h>
#include <include/panic.h>

extern keyboard_status_t ks; //defined in keyboard.c
bool terminal_ready = false;
const char *prompt = "CamOS";
const char prompt_char = '#';
bool command_ready = false;
char *term_command;

void init_terminal(void) {
    if (CLEAR_SCREEN_ON_TERMINAL_START) {
        tty_clear();
    }

    term_command = kmalloc(TERMINAL_COMMAND_LENGTH);
    command_ready = false;
    memclear(term_command, TERMINAL_COMMAND_LENGTH);

    printf("This is a mock shell for test purposes only\n");
    while(true) {
        set_tty_char_fg(RGB(51, 153, 0));
        printf("%s", prompt);
        set_tty_char_fg(TTY_COLOR_WHITE);
        printf("%c ", prompt_char);
        while(!command_ready);
        printf("\n");
        term_switch_command(term_command);
        memclear(term_command, TERMINAL_COMMAND_LENGTH);
        command_ready = false;
    }
}

void term_switch_command(char *comm) {
    char *args = strtok(comm, " ");

    uint8_t program_length = (uint64_t) args - (uint64_t) comm;
    char program[program_length];
    strncpy(program, comm, program_length);
    args++; //skip the space

    if (strncmp(program, "clear", 4) == 0) {
        tty_clear();
    } else if (strncmp(program, "whoami", 6) == 0) {
        printf("user\n");
    } else if (strncmp(program, "identify", 8) == 0) {
        printf("CamOS v1.0\n");
    } else if (strncmp(program, "panic", 5) == 0) {
        panic("test panic scenario\n");
    } else {
        printf("%s: command not found\n", program);
    }
}

/*
called by keypressed() when a terminal control key is pressed on the keyboard
*/
void term_control(uint8_t code) {
    assert_true(IS_TERMINAL_CONTROL(code));

    switch(code) {
        case KEYCODE_LINE_FEED:
            read(STDIN_FILENO, term_command, TERMINAL_COMMAND_LENGTH);
            command_ready = true;
            break;

        case KEYCODE_BACKSPACE:
            tty_backspace();
            read(STDIN_FILENO, null, 1);
            break;

        case KEYCODE_UP_ARROW:
            printf("up arrow\n");
            break;

        case KEYCODE_RIGHT_ARROW:
            printf("right arrow\n");
            break;

        case KEYCODE_DOWN_ARROW:
            printf("down arrow\n");
            break;

        case KEYCODE_LEFT_ARROW:
            printf("left arrow\n");
            break;
    }
}

/*
called by keypressed() when a printable key is pressed on the keyboard.
this function interprets that character based on the keyboard status.
*/
void term_putc(char c) {
    assert_true(IS_PRINTABLE(c));

    if ((ks.capslock || ks.shift) && c >= 'a' && c <= 'z') {
        c -= 32;
    } else if (ks.altgr) {
        switch(c) {
            case 'o':
                c = '@';
                break;

            case 'a':
                c = '#';
                break;
        }
    }

    printf("%c", c);
    write(STDIN_FILENO, &c, 1);
}
