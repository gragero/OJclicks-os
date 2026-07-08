#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PMM_BLOCK_SIZE 4096 
void pmm_init(uint32_t total_memory_bytes);

void pmm_mark_used(uint32_t start_block, uint32_t count);

void* pmm_alloc_block(void);

void pmm_free_block(void* block_address);

uint32_t pmm_get_total_blocks(void);
uint32_t pmm_get_used_blocks(void);
uint32_t pmm_get_free_blocks(void);

#endif
