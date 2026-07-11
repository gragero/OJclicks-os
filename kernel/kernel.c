#include "include/vga.h"
#include "include/string.h"
#include "include/keyboard.h"
#include "include/idt.h"
#include "include/isr.h"
#include "include/pic.h"
#include "include/io.h"
#include "include/timer.h"
#include "include/pmm.h"
#include "include/paging.h"
#include "include/gdt.h"
#include "include/heap.h"
#include "include/task.h"
#include "include/fs.h"
#include "include/usermode.h"
#include "include/syscall.h"
#include "include/elf.h"

#define CMD_BUFFER_SIZE 128

extern uint32_t bss_start;
extern uint32_t bss_end;

static void print_banner(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("=========================================\n");
    vga_puts("   OJclicks OS - Kernel v0.8 (C Edition)\n");
    vga_puts("=========================================\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("Type 'help' for a list of commands.\n\n");
}

static void print_prompt(void) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("ojclicks> ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
}

static void cmd_help(void) {
    vga_puts("Available commands:\n");
    vga_puts("  help        - show this list\n");
    vga_puts("  clear       - clear the screen\n");
    vga_puts("  about       - info about this kernel\n");
    vga_puts("  echo        - print back what you type\n");
    vga_puts("  color       - change text color (color <0-15>)\n");
    vga_puts("  uptime      - show seconds since boot\n");
    vga_puts("  meminfo     - show physical memory stats\n");
    vga_puts("  heapinfo    - show heap allocator stats\n");
    vga_puts("  pgfault     - trigger a test page fault\n");
    vga_puts("  halt        - stop the CPU\n");
    vga_puts("  reboot      - reboot the system\n");
    vga_puts("  oj off      - power off the system\n");
    vga_puts("  uname       - system info\n");
    vga_puts("  beep        - PC speaker beep\n");
    vga_puts("  time        - show timer ticks\n");
    vga_puts("  calc        - simple calculator (calc 2+2)\n");
    vga_puts("  rand        - random number (rand <max>)\n");
    vga_puts("  fib         - fibonacci sequence (fib <n>)\n");
    vga_puts("  primes      - list primes up to n (primes <n>)\n");
    vga_puts("  note        - save a note (note <text>)\n");
    vga_puts("  notes       - list saved notes\n");
    vga_puts("  clearnotes  - delete all saved notes\n");
    vga_puts("  mtdemo      - cooperative multitasking demo\n");
    vga_puts("  ls          - list files on disk\n");
    vga_puts("  write       - save a file (write <name> <content>)\n");
    vga_puts("  read        - read a file (read <name>)\n");
    vga_puts("  usertest    - run a test program in Ring 3\n");
    vga_puts("  runelf      - load and run a real ELF binary from disk\n");
    vga_puts("  snake       - play snake game\n");
}

static void cmd_about(void) {
    vga_puts("OJclicks OS v0.8\n");
    vga_puts("Built from scratch in Assembly + C\n");
    vga_puts("Boot: BIOS -> Real Mode -> A20 -> GDT -> Protected Mode -> C Kernel\n");
    vga_puts("Features: GDT, IDT, ISR, PIC, Timer, PMM, Paging, Heap, VGA, Keyboard\n");
}

static void cmd_echo(const char* args) {
    vga_puts(args);
    vga_putc('\n');
}

static void cmd_color(const char* args) {
    int value = 0;
    int has_digit = 0;
    for (size_t i = 0; args[i] != '\0'; i++) {
        if (args[i] >= '0' && args[i] <= '9') {
            value = value * 10 + (args[i] - '0');
            has_digit = 1;
        }
    }
    if (!has_digit || value < 0 || value > 15) {
        vga_puts("Usage: color <0-15>\n");
        return;
    }
    vga_set_color((vga_color_t)value, VGA_BLACK);
    vga_puts("Color changed.\n");
}

static void cmd_meminfo(void) {
    uint32_t total = pmm_get_total_blocks();
    uint32_t used = pmm_get_used_blocks();
    uint32_t free = pmm_get_free_blocks();
    vga_puts("Physical Memory (4KB blocks):\n");
    vga_puts("  Total: "); vga_put_dec(total); vga_puts(" blocks (");
    vga_put_dec(total * 4); vga_puts(" KB)\n");
    vga_puts("  Used:  "); vga_put_dec(used); vga_puts(" blocks (");
    vga_put_dec(used * 4); vga_puts(" KB)\n");
    vga_puts("  Free:  "); vga_put_dec(free); vga_puts(" blocks (");
    vga_put_dec(free * 4); vga_puts(" KB)\n");
}

static void cmd_heapinfo(void) {
    vga_puts("Kernel Heap:\n");
    vga_puts("  Used: "); vga_put_dec(heap_get_used()); vga_puts(" bytes\n");
    vga_puts("  Free: "); vga_put_dec(heap_get_free()); vga_puts(" bytes\n");
}

static void cmd_pgfault(void) {
    vga_puts("Triggering intentional page fault...\n");
    volatile uint32_t* bad = (uint32_t*)0x4000000;
    *bad = 42;
}

static void cmd_uptime(void) {
    vga_puts("Uptime: ");
    vga_put_dec(timer_get_seconds());
    vga_puts(" seconds\n");
}

static void cmd_halt(void) {
    vga_puts("Halting CPU.\n");
    asm volatile ("cli");
    for (;;) {
        asm volatile ("hlt");
    }
}

static void cmd_reboot(void) {
    vga_puts("Rebooting...\n");
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    for (;;);
}

static void cmd_oj_off(void) {
    vga_puts("OJclicks OS shutting down...\n");
    outw(0x604, 0x2000);
    asm volatile ("cli");
    for (;;) {
        asm volatile ("hlt");
    }
}

static void cmd_uname(void) {
    vga_puts("OJclicks OS v0.7.5\n");
    vga_puts("Kernel: Custom x86 Protected Mode\n");
    vga_puts("Compiler: GCC cross-compiler\n");
    vga_puts("Architecture: i386\n");
}

static void cmd_beep(void) {
    outb(0x43, 0xB6);
    outb(0x42, 1193180 / 1000);
    outb(0x42, (1193180 / 1000) >> 8);
    uint8_t tmp = inb(0x61);
    outb(0x61, tmp | 0x03);
    for (volatile int i = 0; i < 10000000; i++);
    outb(0x61, tmp);
    vga_puts("Beep!\n");
}

static void cmd_time(void) {
    vga_puts("Timer ticks: ");
    vga_put_dec(timer_get_ticks());
    vga_putc('\n');
}

static int k_atoi(const char* s) {
    int res = 0;
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res;
}

static void cmd_calc(const char* args) {
    int a = 0, b = 0;
    char op = '+';
    const char* p = args;

    while (*p >= '0' && *p <= '9') {
        a = a * 10 + (*p - '0');
        p++;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
        op = *p;
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        b = b * 10 + (*p - '0');
        p++;
    }

    int res = 0;
    switch (op) {
        case '+': res = a + b; break;
        case '-': res = a - b; break;
        case '*': res = a * b; break;
        case '/': res = (b != 0) ? a / b : 0; break;
    }

    vga_puts("Result: ");
    vga_put_dec(res);
    vga_putc('\n');
}

static uint32_t seed = 12345;
static void cmd_rand(const char* args) {
    int max = k_atoi(args);
    if (max <= 0) max = 100;
    seed = seed * 1103515245 + 12345;
    uint32_t r = (seed / 65536) % 32768;
    vga_puts("Random: ");
    vga_put_dec(r % max);
    vga_putc('\n');
}

static void cmd_fib(const char* args) {
    int n = k_atoi(args);
    if (n <= 0 || n > 60) {
        vga_puts("Usage: fib <1-60>\n");
        return;
    }
    uint32_t* seq = (uint32_t*)kmalloc(n * sizeof(uint32_t));
    if (!seq) {
        vga_puts("Out of memory.\n");
        return;
    }
    seq[0] = 0;
    if (n > 1) seq[1] = 1;
    for (int i = 2; i < n; i++) {
        seq[i] = seq[i - 1] + seq[i - 2];
    }
    for (int i = 0; i < n; i++) {
        vga_put_dec(seq[i]);
        vga_putc(' ');
    }
    vga_putc('\n');
    kfree(seq);
}

static void cmd_primes(const char* args) {
    int n = k_atoi(args);
    if (n < 2 || n > 2000) {
        vga_puts("Usage: primes <2-2000>\n");
        return;
    }
    uint8_t* sieve = (uint8_t*)kmalloc(n + 1);
    if (!sieve) {
        vga_puts("Out of memory.\n");
        return;
    }
    k_memset(sieve, 1, n + 1);
    sieve[0] = 0;
    sieve[1] = 0;
    for (int i = 2; i * i <= n; i++) {
        if (sieve[i]) {
            for (int j = i * i; j <= n; j += i) {
                sieve[j] = 0;
            }
        }
    }
    for (int i = 2; i <= n; i++) {
        if (sieve[i]) {
            vga_put_dec(i);
            vga_putc(' ');
        }
    }
    vga_putc('\n');
    kfree(sieve);
}

typedef struct note_node {
    char* text;
    struct note_node* next;
} note_node_t;

static note_node_t* notes_head = NULL;

static void cmd_note(const char* args) {
    if (k_strlen(args) == 0) {
        vga_puts("Usage: note <text>\n");
        return;
    }

    note_node_t* node = (note_node_t*)kmalloc(sizeof(note_node_t));
    if (!node) {
        vga_puts("Out of memory.\n");
        return;
    }

    size_t len = k_strlen(args);
    char* text = (char*)kmalloc(len + 1);
    if (!text) {
        kfree(node);
        vga_puts("Out of memory.\n");
        return;
    }

    k_strcpy(text, args);
    node->text = text;
    node->next = notes_head;
    notes_head = node;

    vga_puts("Note saved.\n");
}

static void cmd_notes(void) {
    if (!notes_head) {
        vga_puts("No notes saved.\n");
        return;
    }

    note_node_t* current = notes_head;
    int i = 1;
    while (current) {
        vga_put_dec(i++);
        vga_puts(". ");
        vga_puts(current->text);
        vga_putc('\n');
        current = current->next;
    }
}

static void cmd_clearnotes(void) {
    note_node_t* current = notes_head;
    while (current) {
        note_node_t* next = current->next;
        kfree(current->text);
        kfree(current);
        current = next;
    }
    notes_head = NULL;
    vga_puts("Notes cleared.\n");
}

#define SNAKE_W 40
#define SNAKE_H 20
#define SNAKE_MAX 100

static void cmd_snake(void) {
    vga_clear();
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);

    int sx[SNAKE_MAX], sy[SNAKE_MAX];
    int len = 3, dir = 1, score = 0;
    int fx = 15, fy = 10;

    for (int i = 0; i < len; i++) {
        sx[i] = 20 - i;
        sy[i] = 10;
    }

    vga_puts("=== SNAKE GAME ===\n");
    vga_puts("Use WASD to move, Q to quit\n");

    for (int frame = 0; frame < 5000; frame++) {
        vga_puts("\n");
        for (int y = 0; y < SNAKE_H; y++) {
            for (int x = 0; x < SNAKE_W; x++) {
                int is_snake = 0;
                for (int i = 0; i < len; i++) {
                    if (sx[i] == x && sy[i] == y) {
                        is_snake = 1;
                        break;
                    }
                }
                if (x == fx && y == fy) {
                    vga_putc('*');
                } else if (is_snake) {
                    vga_putc('O');
                } else {
                    vga_putc(' ');
                }
            }
            vga_putc('\n');
        }
        vga_puts("Score: ");
        vga_put_dec(score);
        vga_putc('\n');

        for (volatile int d = 0; d < 8000000; d++);

        char c = 0;
        if (keyboard_has_key()) {
            c = keyboard_get_char();
            if (c == 'q') break;
            if (c == 'w' && dir != 2) dir = 0;
            if (c == 'd' && dir != 3) dir = 1;
            if (c == 's' && dir != 0) dir = 2;
            if (c == 'a' && dir != 1) dir = 3;
        }

        for (int i = len - 1; i > 0; i--) {
            sx[i] = sx[i - 1];
            sy[i] = sy[i - 1];
        }

        if (dir == 0) sy[0]--;
        if (dir == 1) sx[0]++;
        if (dir == 2) sy[0]++;
        if (dir == 3) sx[0]--;

        if (sx[0] < 0) sx[0] = SNAKE_W - 1;
        if (sx[0] >= SNAKE_W) sx[0] = 0;
        if (sy[0] < 0) sy[0] = SNAKE_H - 1;
        if (sy[0] >= SNAKE_H) sy[0] = 0;

        if (sx[0] == fx && sy[0] == fy) {
            if (len < SNAKE_MAX) len++;
            score += 10;
            fx = (fx * 7 + 13) % SNAKE_W;
            fy = (fy * 5 + 7) % SNAKE_H;
        }
    }

    vga_clear();
    vga_puts("Game Over! Score: ");
    vga_put_dec(score);
    vga_putc('\n');
}

static volatile int demo_a_count = 0;
static volatile int demo_b_count = 0;

static void demo_task_a(void) {
    for (int i = 0; i < 5; i++) {
        vga_puts("[Task A] tick\n");
        demo_a_count++;
        task_yield();
    }
    task_exit();
}

static void demo_task_b(void) {
    for (int i = 0; i < 5; i++) {
        vga_puts("[Task B] tick\n");
        demo_b_count++;
        task_yield();
    }
    task_exit();
}

static void cmd_mtdemo(void) {
    vga_puts("Starting cooperative multitasking demo...\n");
    demo_a_count = 0;
    demo_b_count = 0;
    task_create(demo_task_a);
    task_create(demo_task_b);
    while (demo_a_count < 5 || demo_b_count < 5) {
        task_yield();
    }
    vga_puts("Multitasking demo finished.\n");
}

static void cmd_ls(void) {
    char names[24][OJFS_MAX_NAME];
    uint32_t sizes[24];
    int count = fs_list(names, sizes, 24);

    if (count == 0) {
        vga_puts("No files.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        vga_puts(names[i]);
        vga_puts("  (");
        vga_put_dec(sizes[i]);
        vga_puts(" bytes)\n");
    }
}

static void cmd_write(const char* args) {
    char name[OJFS_MAX_NAME];
    int i = 0;
    while (args[i] != ' ' && args[i] != '\0' && i < OJFS_MAX_NAME - 1) {
        name[i] = args[i];
        i++;
    }
    name[i] = '\0';

    if (i == 0 || args[i] != ' ') {
        vga_puts("Usage: write <filename> <content>\n");
        return;
    }

    const char* content = args + i + 1;
    uint32_t len = k_strlen(content);

    if (fs_write(name, (const uint8_t*)content, len) == 0) {
        vga_puts("File saved.\n");
    } else {
        vga_puts("Write failed.\n");
    }
}

static void cmd_read(const char* args) {
    if (k_strlen(args) == 0) {
        vga_puts("Usage: read <filename>\n");
        return;
    }

    char buffer[512];
    int size = fs_read(args, (uint8_t*)buffer, sizeof(buffer) - 1);
    if (size < 0) {
        vga_puts("File not found.\n");
        return;
    }
    buffer[size] = '\0';
    vga_puts(buffer);
    vga_putc('\n');
}

static void user_test_program(void) {
    const char* msg = "Hello from Ring 3!\n";
    asm volatile (
        "mov %0, %%ebx\n\t"
        "mov $1, %%eax\n\t"
        "int $0x80\n\t"
        :
        : "r"(msg)
        : "eax", "ebx"
    );

    asm volatile (
        "mov $2, %%eax\n\t"
        "int $0x80\n\t"
        :
        :
        : "eax"
    );
}

static void cmd_runelf(void) {
    vga_puts("Loading ELF program from disk...\n");
    elf_run_demo();
    vga_puts("Back in Ring 0 (kernel).\n");
}

static void cmd_usertest(void) {
    vga_puts("Entering Ring 3...\n");
    usermode_run(user_test_program);
    vga_puts("Back in Ring 0 (kernel).\n");
}

static const char* match_command(const char* input, const char* command) {
    size_t cmd_len = k_strlen(command);
    if (k_strncmp(input, command, cmd_len) != 0) return 0;
    if (input[cmd_len] != '\0' && input[cmd_len] != ' ') return 0;
    const char* args = input + cmd_len;
    while (*args == ' ') args++;
    return args;
}

static void handle_command(const char* input) {
    const char* args;
    if (k_strlen(input) == 0) return;

    if ((args = match_command(input, "help"))) cmd_help();
    else if ((args = match_command(input, "clear"))) vga_clear();
    else if ((args = match_command(input, "about"))) cmd_about();
    else if ((args = match_command(input, "echo"))) cmd_echo(args);
    else if ((args = match_command(input, "color"))) cmd_color(args);
    else if ((args = match_command(input, "pgfault"))) cmd_pgfault();
    else if ((args = match_command(input, "meminfo"))) cmd_meminfo();
    else if ((args = match_command(input, "heapinfo"))) cmd_heapinfo();
    else if ((args = match_command(input, "uptime"))) cmd_uptime();
    else if ((args = match_command(input, "halt"))) cmd_halt();
    else if ((args = match_command(input, "reboot"))) cmd_reboot();
    else if ((args = match_command(input, "oj off"))) cmd_oj_off();
    else if ((args = match_command(input, "uname"))) cmd_uname();
    else if ((args = match_command(input, "beep"))) cmd_beep();
    else if ((args = match_command(input, "time"))) cmd_time();
    else if ((args = match_command(input, "calc"))) cmd_calc(args);
    else if ((args = match_command(input, "rand"))) cmd_rand(args);
    else if ((args = match_command(input, "fib"))) cmd_fib(args);
    else if ((args = match_command(input, "primes"))) cmd_primes(args);
    else if ((args = match_command(input, "clearnotes"))) cmd_clearnotes();
    else if ((args = match_command(input, "notes"))) cmd_notes();
    else if ((args = match_command(input, "note"))) cmd_note(args);
    else if ((args = match_command(input, "runelf"))) cmd_runelf();
    else if ((args = match_command(input, "usertest"))) cmd_usertest();
    else if ((args = match_command(input, "ls"))) cmd_ls();
    else if ((args = match_command(input, "write"))) cmd_write(args);
    else if ((args = match_command(input, "read"))) cmd_read(args);
    else if ((args = match_command(input, "mtdemo"))) cmd_mtdemo();
    else if ((args = match_command(input, "snake"))) cmd_snake();
    else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_puts("Huh?: ");
        vga_puts(input);
        vga_puts(" | r u sure?\n");
        vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    }
}

void kmain(void) {
    char input_buffer[CMD_BUFFER_SIZE];

    k_memset((void*)&bss_start, 0, (uint32_t)&bss_end - (uint32_t)&bss_start);

    gdt_init();
    vga_init();
    idt_init();
    isr_install();
    pic_remap();
    timer_init(100);
    keyboard_install();
    pmm_init(32 * 1024 * 1024);
    paging_init();
    heap_init();
    task_init();
    fs_init();
    syscall_init();
    asm volatile ("sti");

    print_banner();

    while (1) {
        print_prompt();
        keyboard_read_line(input_buffer, CMD_BUFFER_SIZE);
        handle_command(input_buffer);
    }
}
