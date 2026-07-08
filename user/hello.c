void _start(void) {
    const char* msg = "Hello from a loaded ELF program!\n";

    asm volatile (
        "mov %0, %%ebx\n\t"
        "mov $1, %%eax\n\t"
        "int $0x80\n\t"
        :
        : "r"(msg)
        : "eax", "ebx"
    );

    asm volatile (
        "mov $2, %%eax\n\t"
        "int $0x80\n\t"
        :
        :
        : "eax"
    );

    for (;;);
}
