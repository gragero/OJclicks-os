#include "include/task.h"
#include "include/heap.h"

#define MAX_TASKS 16
#define TASK_STACK_SIZE 4096

extern void task_switch(uint32_t* old_esp, uint32_t new_esp);
extern void task_switch_exit(uint32_t new_esp);

static task_t tasks[MAX_TASKS];
static task_t* current_task = NULL;
static int task_count = 0;

void task_init(void) {
    tasks[0].stack = NULL;
    tasks[0].esp = 0;
    tasks[0].active = 1;
    tasks[0].next = &tasks[0];
    current_task = &tasks[0];
    task_count = 1;
}

void task_create(void (*entry)(void)) {
    if (task_count >= MAX_TASKS) return;

    task_t* t = &tasks[task_count];
    t->stack = (uint8_t*)kmalloc(TASK_STACK_SIZE);
    if (!t->stack) return;

    uint32_t* sp = (uint32_t*)(t->stack + TASK_STACK_SIZE);
    sp -= 5;
    sp[0] = 0;
    sp[1] = 0;
    sp[2] = 0;
    sp[3] = 0;
    sp[4] = (uint32_t)entry;

    t->esp = (uint32_t)sp;
    t->active = 1;

    t->next = current_task->next;
    current_task->next = t;

    task_count++;
}

void task_yield(void) {
    task_t* prev = current_task;
    task_t* next = current_task->next;

    if (next == prev) return;

    current_task = next;
    task_switch(&prev->esp, next->esp);
}

void task_exit(void) {
    task_t* dying = current_task;
    task_t* prev = dying;

    while (prev->next != dying) {
        prev = prev->next;
    }
    prev->next = dying->next;
    current_task = dying->next;

    if (dying->stack) {
        kfree(dying->stack);
    }

    task_switch_exit(current_task->esp);
}

int task_get_count(void) {
    return task_count;
}
