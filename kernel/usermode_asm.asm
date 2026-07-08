[bits 32]

global usermode_enter_asm
global usermode_return_asm

usermode_enter_asm:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, [esp+4]
    mov ebx, [esp+8]
    mov ecx, [esp+12]
    mov edx, [esp+16]

    mov [ecx], esp
    mov [edx], ebp

    push dword 0x23
    push ebx
    pushf
    pop ecx
    or ecx, 0x200
    push ecx
    push dword 0x1B
    push eax
    iret

usermode_return_asm:
    mov eax, [esp+4]
    mov ebp, [esp+8]
    mov esp, eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ret
