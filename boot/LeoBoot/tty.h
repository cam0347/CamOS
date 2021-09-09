#pragma once

#include "types.h"
#define VGA_HEIGHT 25
#define VGA_WIDTH 80
#define VGA_BUFFER_MAX 65536 //16 Kb
#define VGA_VIEWPORT_LIMIT VGA_HEIGHT * VGA_WIDTH
#define CURSOR_START_SCANLINE 0
#define CURSOR_END_SCANLINE 15

enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15
};

//prototypes
void clear_vga();
void print(const char *str, enum vga_color color);
void println(const char *str, enum vga_color color);
void print_int(int n, enum vga_color color);
void println_int(int n, enum vga_color color);
void print_hex(int n, enum vga_color color);
void println_hex(int n, enum vga_color color);
void error(const char *str);
void int_log(const char *str, uint8_t isr_n);
void vga_scrolldown();
void vga_scrollup();
uint32_t strlen(const char *str);
void aux_push(uint32_t data);
uint32_t aux_pop();
void disable_cursor();
void enable_cursor();
void set_cursor(uint8_t x, uint8_t y);