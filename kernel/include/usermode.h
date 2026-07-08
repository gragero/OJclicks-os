#ifndef USERMODE_H
#define USERMODE_H

void usermode_run(void (*entry)(void));
void usermode_exit(void);

#endif
