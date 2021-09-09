#include "tty.h"
#include "types.h"
#include "low_level.h"

static char vga[VGA_BUFFER_MAX];
static enum vga_color vga_colors[VGA_BUFFER_MAX]; //color array
static uint32_t vga_index = 0, viewport = 0;
static const char *video_mem = (char *) 0xB8000; //rodata section

static void vga_buffer_append(const char *str, enum vga_color color) {
  uint64_t length = strlen(str);
  uint32_t index = vga_index;

  for (uint64_t i = 0; i < length; i++) {
    if (*(str + i) == '\n') {
      for (int j = 0; j < VGA_WIDTH - index % VGA_WIDTH; j++) {
        vga[index + j] = ' ';
        vga_colors[index + j] = 0x00;
      }

      index += VGA_WIDTH - index % VGA_WIDTH;
    } else {
      vga[index] = *(str + i);
      vga_colors[index] = (char) color;
      index++;
    }
  }

  vga_index = index;
}

//writes on the screen the viewport range
static void vga_refresh() {
  uint8_t row = 0, col = 0;

  for (int i = 0; i < VGA_VIEWPORT_LIMIT; i++) {
    *((char *) video_mem + i * 2) = vga[viewport + i];
    *((char *) video_mem + i * 2 + 1) = (char) vga_colors[viewport + i];
  }
}

//set the viewport (if it doesn't go beyond VGA_BUFFER_MAX)
static void set_viewport(const uint32_t row) {
  if (row * VGA_WIDTH < VGA_BUFFER_MAX) {
    viewport = row * VGA_WIDTH;
  }
}

//adds VGA_WIDTH to viewport - max viewport: VGA_BUFFER_MAX - VGA_VIEWPORT_LIMIT
void vga_scrolldown() {
  if (viewport < VGA_BUFFER_MAX - VGA_VIEWPORT_LIMIT) {
    viewport += VGA_WIDTH;
  }

  vga_refresh();
}

//subtracts VGA_WIDTH to viewport - min viewport: 0
void vga_scrollup() {
  if (viewport >= VGA_WIDTH) {
    viewport -= VGA_WIDTH; 
  }

  vga_refresh();
}

//adds text to VGA and if it doesn't fit into the viewport, scrolls down
void print(const char *str, enum vga_color color) {
  vga_buffer_append(str, color);
  vga_refresh();

  while (vga_index >= viewport + VGA_VIEWPORT_LIMIT) {
    vga_scrolldown();
  }
}

void println(const char *str, enum vga_color color) {
  print(str, color);
  print("\n", 0);
}

//clear vga viewport
void clear_vga() {
  for (int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
    //*((char *) video_mem + i) = 0x00;
    *((char *) video_mem + viewport + i) = 0x00;
  }

  vga_refresh();
}

void print_int(int n, enum vga_color color) {
  if (n < 0) {
    print("-", color);
    n *= -1;
  } else if (n == 0) {
    print("0", color);
    return;
  }

  int div = 1, tmp, i = 0;
  char ch;
  char digits[11]; //max number of digits representable with an int (2^32)

  for (int i = 0; i < 11; i++) {
    digits[i] = 0x00;
  }

  while(n / div != 0) {
    tmp = (n / div) % 10;
    ch = 48 + tmp;
    digits[9 - i] = ch;
    div *= 10;
    i++;
  }

  print(&digits[9 - i + 1], color);
}

void println_int(int n, enum vga_color color) {
  print_int(n, color);
  print("\n", VGA_COLOR_WHITE);
}

void print_hex(int n, enum vga_color color) {
  const char conv[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  char str[3] = {0x00, 0x00, 0x00};
  uint32_t i = 0;
  bool neg = n < 0;

  if (neg) {
    n *= -1;
  }

  do {
    aux_push((uint32_t)(n % 16));
    n /= 16;
    i++;
  } while(n != 0);

  if (neg) {
    str[0] = '-';
    print(str, color);
  }

  str[0] = '0';
  str[1] = 'x';
  print(str, color);
  str[1] = 0x00;

  for (int j = 0; j < i; j++) {
    str[0] = conv[aux_pop()];
    print(str, color);
  }
}

void println_hex(int n, enum vga_color color) {
  print_hex(n, color);
  print("\n", VGA_COLOR_WHITE);
}

void error(const char *str) {
  println(str, VGA_COLOR_LIGHT_RED);
}

//when an interrupt is fired this function prints the description
void int_log(const char *str, uint8_t isr_n) {
  print("\n[int ", VGA_COLOR_LIGHT_RED);
  print_int(isr_n, VGA_COLOR_LIGHT_RED);
  print("]: ", VGA_COLOR_LIGHT_RED);
  println(str, VGA_COLOR_LIGHT_RED);
}

void disable_cursor() {
  outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void enable_cursor() {
  outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | CURSOR_START_SCANLINE);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | CURSOR_START_SCANLINE);
}

void set_cursor(uint8_t x, uint8_t y) {
  uint8_t offset = y * VGA_WIDTH + x;
  
  outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (offset & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((offset >> 8) & 0xFF));
}