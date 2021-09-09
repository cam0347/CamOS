/*
ZAVATTARO CAMILLO
27/04/2021

video vga functions headers
*/

#define VIDEOH 1

#ifndef TYPESH
  #include "../types.h"
#endif

#ifndef VGACOLORSH
  #include "vga_colors.h"
#endif

#ifndef STRINGH
  #include "../string.h"
#endif

#define VGA_HEIGHT 50
#define VGA_WIDTH 80

long print(char *str, enum vga_color color);
long println(char *str, enum vga_color color);
void print_int(int n, enum vga_color color);
void rainbow_print(char *str);
void clear_vga();

long print(char *str, enum vga_color color) {
  char *video_mem = (char *) 0xB8000;
  static int row = 0;
  static int row_offset = 0;
  int i = 0;

  while (*(str + i) != 0x0) {
    if ((*(str + i) == '\n') || (row_offset / 2 == VGA_WIDTH)) {
      row++;
      row_offset = 0;
    } else {
      *(video_mem + 2 * row * VGA_WIDTH + row_offset) = *(str + i);
      *(video_mem + 2 * row * VGA_WIDTH + row_offset + 1) = color;
      row_offset += 2;
    }

    i++;
  }

  return i;
}


long println(char *str, enum vga_color color) {
  long ret = print(str, color);
  print("\n", VGA_COLOR_WHITE);
  return ret;
}

//reset video memory
void clear_vga() {
  char *video_mem = (char *) 0xB8000;
  for (int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
    *(video_mem + i) = 0x0;
  }
}

void print_int(int n, enum vga_color color) {
  if (n < 0) {
    print("-", color);
    n *= -1;
  } else if (n == 0) {
    println("0", color);
    return;
  }

  int div = 1, tmp;
  char ch[2] = {0, 0};

  while(n / div != 0) {
    tmp = n / div % 10;
    ch[0] = 48 + tmp;
    print(ch, color);
    div *= 10;
  }

  print("\n", color);
}

void rainbow_print(char *str) {
  char _str[strlen(str)];

  for (long i = 0; i < strlen(str); i++) {
    strcpy(_str, str);
    _str[i + 1] = 0x00;
    print(_str + i, (enum vga_color) (i + 1) % 15);
  }
}