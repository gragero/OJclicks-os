#include "include/syscall.h"
#include "include/isr.h"
#include "include/vga.h"
#include "include/usermode.h"

#define SYS_WRITE 1
#define SYS_EXIT  2

static void syscall_handler(struct registers* regs) {
    switch (regs->eax) {
        case SYS_WRITE:
            vga_puts((const char*)regs->ebx);
            break;
        case SYS_EXIT:
            usermode_exit();
            break;
        default:
            vga_puts("\nUnknown syscall\n");
            break;
    }
}

void syscall_init(void) {
    register_interrupt_handler(128, syscall_handler);
}
