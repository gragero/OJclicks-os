#include "include/elf.h"
#include "include/ata.h"
#include "include/heap.h"
#include "include/string.h"
#include "include/usermode.h"
#include "include/vga.h"

#define ELF_DEMO_LBA 500
#define ELF_DEMO_SECTORS 20

typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf32_ehdr_t;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed)) elf32_phdr_t;

#define PT_LOAD 1

int elf_load_and_run(const uint8_t* image) {
    const elf32_ehdr_t* ehdr = (const elf32_ehdr_t*)image;

    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        vga_puts("Not a valid ELF file.\n");
        return -1;
    }

    const elf32_phdr_t* phdr = (const elf32_phdr_t*)(image + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            uint8_t* dest = (uint8_t*)phdr[i].p_vaddr;
            k_memcpy(dest, image + phdr[i].p_offset, phdr[i].p_filesz);
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                k_memset(dest + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }

    vga_puts("ELF loaded. Jumping to entry point...\n");

    void (*entry)(void) = (void (*)(void))ehdr->e_entry;
    usermode_run(entry);

    return 0;
}

void elf_run_demo(void) {
    uint8_t* buffer = (uint8_t*)kmalloc(ELF_DEMO_SECTORS * 512);
    if (!buffer) {
        vga_puts("Out of memory.\n");
        return;
    }

    for (int i = 0; i < ELF_DEMO_SECTORS; i++) {
        ata_read_sector(ELF_DEMO_LBA + i, buffer + i * 512);
    }

    elf_load_and_run(buffer);

    kfree(buffer);
}
