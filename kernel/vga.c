#include "include/vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)

static size_t cursor_row;
static size_t cursor_col;
static uint8_t current_color;
static uint16_t* const vga_buffer = VGA_MEMORY;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

static inline uint8_t vga_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

void vga_init(void) {
    cursor_row = 0;
    cursor_col = 0;
    current_color = vga_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', current_color);
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    current_color = vga_color(fg, bg);
}

static void vga_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
    }
    cursor_row = VGA_HEIGHT - 1;
}

void vga_putc(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            const size_t index = cursor_row * VGA_WIDTH + cursor_col;
            vga_buffer[index] = vga_entry(' ', current_color);
        }
    } else {
        const size_t index = cursor_row * VGA_WIDTH + cursor_col;
        vga_buffer[index] = vga_entry(c, current_color);
        cursor_col++;
    }

    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
    }

    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_puts(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        vga_putc(str[i]);
    }
}

void vga_put_hex(uint32_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    vga_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (value >> i) & 0xF;
        vga_putc(hex_chars[nibble]);
    }
}

void vga_put_dec(uint32_t value) {
    char buffer[11]; 
    int i = 0;

    if (value == 0) {
        vga_putc('0');
        return;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        vga_putc(buffer[--i]);
    }
}
