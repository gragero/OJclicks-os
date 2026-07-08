#ifndef ELF_H
#define ELF_H

#include <stdint.h>

int elf_load_and_run(const uint8_t* image);
void elf_run_demo(void);

#endif
