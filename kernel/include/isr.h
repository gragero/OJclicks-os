#ifndef ISR_H
#define ISR_H

#include <stdint.h>

struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

typedef void (*isr_t)(struct registers*);

void isr_install(void);
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif
