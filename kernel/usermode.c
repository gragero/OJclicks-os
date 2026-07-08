#include "include/usermode.h"
#include "include/gdt.h"
#include <stdint.h>

#define USER_STACK_SIZE 4096

static uint8_t user_stack[USER_STACK_SIZE];
static uint32_t saved_esp;
static uint32_t saved_ebp;

extern void usermode_enter_asm(uint32_t entry, uint32_t user_stack, uint32_t* saved_esp, uint32_t* saved_ebp);
extern void usermode_return_asm(uint32_t saved_esp, uint32_t saved_ebp);

void usermode_run(void (*entry)(void)) {
    tss_set_kernel_stack(0x90000);

    uint32_t esp_top = (uint32_t)(user_stack + USER_STACK_SIZE);
    usermode_enter_asm((uint32_t)entry, esp_top, &saved_esp, &saved_ebp);
}

void usermode_exit(void) {
    usermode_return_asm(saved_esp, saved_ebp);
}
