#include "include/timer.h"
#include "include/isr.h"
#include "include/io.h"

#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_PORT  0x43
#define PIT_BASE_FREQUENCY 1193182 

static volatile uint32_t tick_count = 0;
static uint32_t configured_frequency = 100;

static void timer_callback(struct registers* regs) {
    (void)regs;
    tick_count++;
}

void timer_init(uint32_t frequency) {
    configured_frequency = frequency;

    register_interrupt_handler(32, timer_callback); 
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_COMMAND_PORT, 0x36);

    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

uint32_t timer_get_seconds(void) {
    return tick_count / configured_frequency;
}

void timer_sleep(uint32_t ms) {
    uint32_t target_ticks = tick_count + (ms * configured_frequency) / 1000;

    while (tick_count < target_ticks) {
        asm volatile ("sti; hlt");
    }
}
