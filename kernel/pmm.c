#include "include/pmm.h"
#include "include/string.h"

#define MAX_BLOCKS (128 * 1024 * 1024 / PMM_BLOCK_SIZE)
#define BITMAP_SIZE (MAX_BLOCKS / 8)

static uint8_t memory_bitmap[BITMAP_SIZE];
static uint32_t total_blocks = 0;
static uint32_t used_blocks_count = 0;

extern uint32_t kernel_end;

static inline void bitmap_set(uint32_t bit) {
    memory_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_clear(uint32_t bit) {
    memory_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int bitmap_test(uint32_t bit) {
    return memory_bitmap[bit / 8] & (1 << (bit % 8));
}

static int32_t bitmap_first_free(void) {
    uint32_t total_bytes = total_blocks / 8;
    for (uint32_t i = 0; i < total_bytes; i++) {
        if (memory_bitmap[i] != 0xFF) {
            for (int j = 0; j < 8; j++) {
                if (!(memory_bitmap[i] & (1 << j))) {
                    return (int32_t)(i * 8 + j);
                }
            }
        }
    }
    return -1; }

void pmm_mark_used(uint32_t start_block, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        uint32_t block = start_block + i;
        if (block < total_blocks && !bitmap_test(block)) {
            bitmap_set(block);
            used_blocks_count++;
        }
    }
}

void pmm_init(uint32_t total_memory_bytes) {
    total_blocks = total_memory_bytes / PMM_BLOCK_SIZE;
    if (total_blocks > MAX_BLOCKS) {
        total_blocks = MAX_BLOCKS;
    }

    k_memset(memory_bitmap, 0, sizeof(memory_bitmap));
    used_blocks_count = 0;

    pmm_mark_used(0, 1);

    uint32_t kernel_start_addr = 0x1000;
    uint32_t kernel_end_addr = (uint32_t)&kernel_end;
    uint32_t kernel_size = kernel_end_addr - kernel_start_addr;
    uint32_t kernel_blocks = (kernel_size / PMM_BLOCK_SIZE) + 1;

    pmm_mark_used(kernel_start_addr / PMM_BLOCK_SIZE, kernel_blocks);
}

void* pmm_alloc_block(void) {
    int32_t block = bitmap_first_free();
    if (block == -1) {
        return 0; 
    }
    bitmap_set((uint32_t)block);
    used_blocks_count++;
    return (void*)((uint32_t)block * PMM_BLOCK_SIZE);
}

void pmm_free_block(void* block_address) {
    uint32_t block = (uint32_t)block_address / PMM_BLOCK_SIZE;
    if (block < total_blocks && bitmap_test(block)) {
        bitmap_clear(block);
        used_blocks_count--;
    }
}

uint32_t pmm_get_total_blocks(void) {
    return total_blocks;
}

uint32_t pmm_get_used_blocks(void) {
    return used_blocks_count;
}

uint32_t pmm_get_free_blocks(void) {
    return total_blocks - used_blocks_count;
}
