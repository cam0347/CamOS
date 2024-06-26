#include <include/types.h>
#include <include/mem.h>
#include <tty/include/tty.h>
#include <tty/include/psf_font.h>
#include <tty/include/vsnprintf.h>
#include <tty/include/hue.h>
#include <include/string.h>
#include <include/low_level.h>
#include <include/panic.h>

void *fb, *glyphs;
uint64_t tty_height, tty_width, tty_size;

struct {
    uint64_t x;
    uint64_t y;
} tty_cursor;

struct {
    uint64_t height;
    uint64_t width;
    uint8_t cell_height;
    uint8_t cell_width;
    uint32_t cell_pitch;
} tty_grid;

/* this is the default background and foreground for text */
struct {
    tty_color_t foreground;
    tty_color_t background;
    bool bg_transparent;
} tty_color;

bool tty_ready = false;

//initialize terminal
bool init_tty(struct leokernel_boot_params *bp) {
    if (tty_ready) {
        return false;
    }

    if (!tty_load_font(bp->font, &glyphs, &tty_grid.cell_height, &tty_grid.cell_width)) {
        //what happens now?
        return false;
    }

    fb = bp->frame_buffer;

    tty_height = bp->video_height;
    tty_width = bp->video_width;
    tty_size = bp->frame_buffer_size;

    tty_color.foreground = TTY_COLOR_WHITE;
    tty_color.background = TTY_COLOR_BLACK;
    tty_color.bg_transparent = true;

    tty_grid.cell_pitch = bp->video_pitch;
    tty_grid.width = tty_width / tty_grid.cell_width;
    tty_grid.height = tty_height / tty_grid.cell_height;

    tty_cursor.x = 0;
    tty_cursor.y = 0;

    tty_clear();
    tty_ready = true;

    return true;
}

bool tty_load_font(void *file, void **glyphs, uint8_t *glyph_height, uint8_t *glyph_width) {
    if (!file) {
        return false;
    }

    bool psf1 = (*(uint16_t *) file == PSF1_HEADER_MAGIC);
    bool valid = psf1 || (*(uint32_t *) file == PSF2_HEADER_MAGIC);

    if (!valid) {
        return false;
    }

    if (psf1) {
        psf1_header *header = (psf1_header *) file;
        uint8_t size = header->character_size; //number of bytes per glyph, also glyph height (the width is always 8 pixels)
        *glyph_height = size;
        *glyph_width = 8;
        *glyphs = file + sizeof(psf1_header);
    } else {
        psf2_header *header = (psf2_header *) file;
        *glyph_height = (uint8_t) header->glyph_height /*+ (8 - header->glyph_height % 8) % 8*/; //rounds to the nearest greater multiple of 8
        *glyph_width = (uint8_t) header->glyph_width + (8 - header->glyph_width % 8) % 8;
        *glyphs = file + sizeof(psf2_header);
        //*glyph_padding = (8 - header->glyph_width % 8) % 8;
        //*glyph_padding = *glyph_padding == 0 ? 1 : *glyph_padding;
    }

    return true;
}

void set_tty_char_fg(tty_color_t new_color) {
    tty_color.foreground = new_color;
}

tty_color_t get_tty_char_fg() {
    return tty_color.foreground;
}

void set_tty_char_bg(tty_color_t new_color) {
    tty_color.background = new_color;
}

tty_color_t get_tty_char_bg() {
    return tty_color.background;
}

uint64_t get_tty_grid_height() {
    return tty_grid.height;
}

uint64_t get_tty_grid_width() {
    return tty_grid.width;
}

void tty_enable_bg() {
    tty_color.bg_transparent = false;
}

void tty_disable_bg() {
    tty_color.background = true;
}

inline void plot_pixel(uint64_t x, uint64_t y, tty_color_t pixel) {
    if (!tty_ready) {return;}
    uint32_t *loc = fb + y * tty_grid.cell_pitch * 4 + x * 4;
    *loc = pixel;
}

void launch_splashscreen() {
    uint16_t hue = 0;

    for (uint64_t y = 0; y < tty_height; y++) {
        for (uint64_t x = 0; x < tty_width; x++) {
            uint8_t r, g, b;
            hsv_to_rgb(hue % 360, 1, 1, &r, &g, &b);
            uint32_t rgb = r << 16 | g << 8 | b;
            plot_pixel(x, y, rgb);

            for (int i = 0; i < 2500; i++) {}
        }

        hue++;
    }
}

//print a character to a specific grid location
void putchar_at(char c, uint32_t x, uint32_t y, tty_color_t fg, tty_color_t bg) {
    if (!tty_ready) {return;}

    if (c == '\t') {
        c = ' ';
    }

    uint8_t *glyph = glyphs + tty_grid.cell_height * (tty_grid.cell_width / 8) * c;

    for (int i = 0; i < tty_grid.cell_height; i++) {
        for (int j = 0; j < tty_grid.cell_width; j++) {
            if (*(glyph + i * tty_grid.cell_width / 8 + j / 8) >> (7 - j % 8) & 1) {
                plot_pixel(x + j, y + i, fg);
            } else if (!tty_color.bg_transparent) {
                plot_pixel(x + j, y + i, bg);
            }
        }
    }
}

void putchar(char c, tty_color_t fg, tty_color_t bg) {
    if (!tty_ready) {
        return;
    }

    //if we reached the end of line or the character to be displayed is a new line, move the cursor to the next line full left
    if (c == '\n' || tty_cursor.x == tty_grid.width) {
        tty_cursor.x = 0;

        if (tty_cursor.y < tty_grid.height) {
            tty_cursor.y++;
        } else {
            tty_clear();
            tty_cursor.x = 0;
            tty_cursor.y = 0;
        }

        if (c == '\n') {
            return;
        }
    }

    putchar_at(c, tty_grid.cell_width * tty_cursor.x, tty_grid.cell_height * tty_cursor.y, fg, bg);
    tty_cursor.x++; //move the cursor 1 position right
}

/* clears a grid cell */
void tty_clear_cell(uint32_t x, uint32_t y) {
    if (x >= tty_grid.width || y >= tty_grid.height) {
        return;
    }

    for (uint8_t i = 0; i < tty_grid.cell_height; i++) {
        for (uint8_t j = 0; j < tty_grid.cell_width; j++) {
            plot_pixel(x * tty_grid.cell_width + j, y * tty_grid.cell_height + i, tty_color.background);
        }
    }
}

void tty_backspace() {
    if (tty_cursor.x == 0) {
        if (tty_cursor.y > 0) {
            tty_cursor.x = tty_grid.width - 1;
            tty_cursor.y--;
        }
    } else {
        tty_cursor.x--;
    }

    tty_clear_cell(tty_cursor.x, tty_cursor.y);
}

void print_color(char *str, tty_color_t fg, tty_color_t bg) {
    if (!tty_ready) {return;}
    uint32_t len = strlen(str);

    for (int i = 0; i < len; i++) {
        char c = *(str + i);

        if (c >= 32 && c <= 126 || c == '\n' || c == '\t') {
            putchar(c, fg, bg);
        }
    }
}

void printf(const char *fmt, ...) {
    if (!tty_ready) {return;}
    va_list args;
    va_start(args, fmt);
    char buf[PRINTF_MAX_BUFFER];
    vsnprintf(buf, PRINTF_MAX_BUFFER, (char *) fmt, args);
    print_color(buf, tty_color.foreground, tty_color.background);
    va_end(args);
}

/* clears the screen */
void tty_clear() {
    memclear(fb, tty_size);
    tty_cursor.x = 0;
    tty_cursor.y = 0;
}

/* prints an error message and halt */
void fail(char *str) {
    printf("\n");
    for (uint16_t i = 0; i < tty_grid.width; i++) {
        printf("-");
    }

    printf("system failure - %s\n", str);

    for (uint16_t i = 0; i < tty_grid.width; i++) {
        printf("-");
    }

    char panic_msg[100] = "system invoked failure: ";

    if (strlen(panic_msg) + strlen(str) <= 100) {
        strcat(panic_msg, str);
    } else {
        strncpy(panic_msg + strlen(panic_msg), str, 100 - strlen(panic_msg));
    }

    panic(panic_msg);
}