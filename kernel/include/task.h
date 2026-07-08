#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct task {
    uint32_t esp;
    uint8_t* stack;
    struct task* next;
    int active;
} task_t;

void task_init(void);
void task_create(void (*entry)(void));
void task_yield(void);
void task_exit(void);
int task_get_count(void);

#endif
