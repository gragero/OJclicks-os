#include "include/keyboard.h"
#include "include/io.h"
#include "include/vga.h"
#include "include/isr.h"

#define KB_DATA_PORT 0x60
#define KB_STATUS_PORT 0x64
#define KB_BUFFER_SIZE 256

static const char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,
    '*',
    0,
    ' ',
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static volatile char kb_buffer[KB_BUFFER_SIZE];
static volatile int kb_read_pos = 0;
static volatile int kb_write_pos = 0;

static void keyboard_callback(struct registers* regs) {
    (void)regs;

    uint8_t scancode = inb(KB_DATA_PORT);

    if (scancode & 0x80) {
        return;
    }

    char c = scancode_to_ascii[scancode];
    if (c == 0) {
        return;
    }

    int next_write = (kb_write_pos + 1) % KB_BUFFER_SIZE;
    if (next_write != kb_read_pos) {
        kb_buffer[kb_write_pos] = c;
        kb_write_pos = next_write;
    }
}

void keyboard_install(void) {
    register_interrupt_handler(33, keyboard_callback);
}

char keyboard_getchar(void) {
    while (kb_read_pos == kb_write_pos) {
        asm volatile ("sti; hlt");
    }
    char c = kb_buffer[kb_read_pos];
    kb_read_pos = (kb_read_pos + 1) % KB_BUFFER_SIZE;
    return c;
}

void keyboard_read_line(char* buffer, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        char c = keyboard_getchar();

        if (c == '\n') {
            vga_putc('\n');
            break;
        }

        if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putc('\b');
            }
            continue;
        }

        buffer[i++] = c;
        vga_putc(c);
    }
    buffer[i] = '\0';
}

int keyboard_has_key(void) {
    return kb_read_pos != kb_write_pos;
}

char keyboard_get_char(void) {
    if (kb_read_pos == kb_write_pos) {
        return 0;
    }
    char c = kb_buffer[kb_read_pos];
    kb_read_pos = (kb_read_pos + 1) % KB_BUFFER_SIZE;
    return c;
}
