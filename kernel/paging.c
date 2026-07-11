#include "include/paging.h"
#include "include/isr.h"
#include "include/vga.h"

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

extern void paging_enable(uint32_t page_directory_physical_addr);

static void page_fault_handler(struct registers* regs) {
    uint32_t faulting_address;
    asm volatile ("mov %%cr2, %0" : "=r"(faulting_address));

    int present  = !(regs->err_code & 0x1); 
    int write    = regs->err_code & 0x2;    
    int user     = regs->err_code & 0x4;    

    vga_set_color(VGA_WHITE, VGA_RED);
    vga_puts("\n\n  PAGE FAULT\n");
    vga_puts("  Address: ");
    vga_put_hex(faulting_address);
    vga_puts("\n  Reason:  ");
    vga_puts(present ? "page not present, " : "protection violation, ");
    vga_puts(write ? "write access, " : "read access, ");
    vga_puts(user ? "from user mode\n" : "from kernel mode\n");
    vga_puts("  System halted.\n");

    asm volatile ("cli");
    for (;;) {
        asm volatile ("hlt");
    }
}

void paging_init(void) {
    for (int i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0 | PAGE_RW | PAGE_USER; 
    }

    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

    register_interrupt_handler(14, page_fault_handler);

    paging_enable((uint32_t)page_directory);
}
