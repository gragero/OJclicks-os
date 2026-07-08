[bits 32]

section .text
    extern kmain    
    global _start

_start:
    call kmain       

    cli
.hang:
    hlt
    jmp .hang
