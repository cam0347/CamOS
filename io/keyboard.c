#include <io/include/keyboard.h>
#include <include/types.h>
#include <io/include/files.h>
#include <int/include/apic.h>
#include <tty/include/term.h>
#include <int/include/int.h>
#include <tty/include/tty.h>

keyboard_status_t ks;
bool keyboard_ready = false;

const char keyboard_codes[] = {
    KEYCODE_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\'', 'i', KEYCODE_BACKSPACE, KEYCODE_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',
    'p', 'e', '+', KEYCODE_LINE_FEED, KEYCODE_LCTRL_DOWN, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'o', 'a', 'u', KEYCODE_LSHIFT_DOWN, '<', 'z', 'x', 'c',
    'v', 'b', 'n', 'm', ',', '.', '-', KEYCODE_RSHIFT_DOWN, '*', KEYCODE_ALT_DOWN, ' ', KEYCODE_CAPS_LOCK, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3',
    '0', '.', KEYCODE_LCTRL_UP, KEYCODE_LSHIFT_UP, KEYCODE_RSHIFT_UP, KEYCODE_ALT_UP
};

/* initialize keyboard input subsystem */
bool init_keyboard(void) {
    ks.capslock = false;
    ks.numlock = false;
    ks.scroll_lock = false;
    ks.shift = false;
    ks.ctrl = false;
    ks.altgr = false;
    ks.alt = false;
    ks.fn = false;
    ks.source = ps2; //by default the keyboard input is set to be ps/2, when and if a usb keyboard is found it will be changed
    apic_irq(1, 0x18, fixed, physical, active_high, edge, false, 0); //ps2 irq
    keyboard_ready = true;
}

/*
this function is called by keyboard drivers.
if it's a printable character calls the appropriate function,
if it's a control character calls the function that handles the controlled device (keyboard or terminal).
if the code is 0 it's invalid, return.
*/
void keypressed(uint8_t code) {
    if (!keyboard_ready || code == 0) {
        return;
    }

    if (IS_PRINTABLE(code)) {
        /*
        calls the terminal function to pass the character.
        that function has the responsibility to check whether the printable character
        is part of a keybinding (checks the keyboard status
        for active keybinding triggers (shift, ctrl, alt, altgr)),
        and to interpret the character being pressed (if a alt or capslock is active, altgr and so on)
        */
       term_putc(code);
    } else if (IS_KEYBOARD_CONTROL(code)) {
        keyboard_control(code);
    } else if (IS_TERMINAL_CONTROL(code)) {
        term_control(code);
    }
}

/* edits keyboard status (keys interpretation) */
void keyboard_control(uint8_t control) {
    switch(control) {
        case KEYCODE_LSHIFT_DOWN:
        case KEYCODE_RSHIFT_DOWN:
            ks.shift = true;
            break;

        case KEYCODE_LSHIFT_UP:
        case KEYCODE_RSHIFT_UP:
            ks.shift = false;
            break;

        case KEYCODE_LCTRL_DOWN:
        case KEYCODE_RCTRL_DOWN:
            ks.ctrl = true;
            break;

        case KEYCODE_LCTRL_UP:
        case KEYCODE_RCTRL_UP:
            ks.ctrl = false;
            break;

        case KEYCODE_ALT_DOWN:
            ks.alt = true;
            break;

        case KEYCODE_ALT_UP:
            ks.alt = false;
            break;

        case KEYCODE_ALTGR_DOWN:
            ks.altgr = true;
            break;

        case KEYCODE_ALTGR_UP:
            ks.altgr = false;
            break;

        case KEYCODE_CAPS_LOCK:
            ks.capslock = !ks.capslock;
            break;
    }
}

/* waits for the user to press a key */
bool keyboard_wait_in_progress = false;
void keyboard_wait(char *msg) {
    if (msg) {
        printf("%s\n", msg);
    }

    keyboard_wait_in_progress = true;
    uint8_t hook_n = int_hook(0x18, (void *) &keyboard_wait_handler);
    printf("hook n %d\n", hook_n);
    while(keyboard_wait_in_progress);
    int_unhook(0x18, hook_n);
}

void keyboard_wait_handler() {
    keyboard_wait_in_progress = false;
}