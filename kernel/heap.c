#include "include/heap.h"
#include "include/pmm.h"
#include "include/string.h"

typedef struct block_header {
    uint32_t size;
    uint8_t free;
    struct block_header* next;
} block_header_t;

static block_header_t* free_list = NULL;
static uint32_t heap_used_bytes = 0;
static uint32_t heap_total_bytes = 0;

static block_header_t* request_space(block_header_t* last, uint32_t size) {
    uint32_t total_needed = size + sizeof(block_header_t);
    uint32_t pages = (total_needed + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    if (pages < 1) pages = 1;

    void* mem = pmm_alloc_block();
    if (!mem) return NULL;

    for (uint32_t i = 1; i < pages; i++) {
        pmm_alloc_block();
    }

    block_header_t* block = (block_header_t*)mem;
    block->size = pages * PMM_BLOCK_SIZE - sizeof(block_header_t);
    block->free = 0;
    block->next = NULL;

    if (last) {
        last->next = block;
    }

    heap_total_bytes += pages * PMM_BLOCK_SIZE;
    return block;
}

void heap_init(void) {
    free_list = NULL;
    heap_used_bytes = 0;
    heap_total_bytes = 0;
}

static block_header_t* find_free_block(block_header_t** last, uint32_t size) {
    block_header_t* current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        *last = current;
        current = current->next;
    }
    return NULL;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = (size + 3) & ~3u;

    block_header_t* block;
    block_header_t* last = free_list;

    if (!free_list) {
        block = request_space(NULL, size);
        if (!block) return NULL;
        free_list = block;
    } else {
        block = find_free_block(&last, size);
        if (!block) {
            block = request_space(last, size);
            if (!block) return NULL;
        } else {
            block->free = 0;
        }
    }

    heap_used_bytes += block->size;
    return (void*)(block + 1);
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_header_t* block = (block_header_t*)ptr - 1;
    block->free = 1;
    if (heap_used_bytes >= block->size) {
        heap_used_bytes -= block->size;
    }
}

uint32_t heap_get_used(void) {
    return heap_used_bytes;
}

uint32_t heap_get_free(void) {
    return heap_total_bytes - heap_used_bytes;
}
