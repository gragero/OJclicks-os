[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000

start:
    mov si, MSG_REAL
    call print_string_rm

    call load_kernel
    call enable_a20
    call load_gdt

    cli
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp 0x08:protected_mode_start

print_string_rm:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0e
    int 0x10
    jmp print_string_rm
.done:
    ret

load_kernel:
    mov bx, KERNEL_OFFSET
    mov dh, 40
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

disk_load:
    push dx
    mov ah, 0x02
    mov al, dh
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    int 0x13
    jc disk_error
    pop dx
    cmp al, dh
    jne disk_error
    ret

disk_error:
    mov si, DISK_ERR_MSG
    call print_string_rm
    jmp $

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

load_gdt:
    lgdt [gdt_descriptor]
    ret

gdt_start:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BOOT_DRIVE db 0
MSG_REAL db 'Real Mode: A20 + GDT loading...', 0
DISK_ERR_MSG db 'Disk read error!', 0

[bits 32]
protected_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x90000

    call clear_screen_pm

    jmp KERNEL_OFFSET

clear_screen_pm:
    mov edi, 0xb8000
    mov ecx, 80*25
    mov ax, 0x0f20
.clear_loop:
    mov [edi], ax
    add edi, 2
    dec ecx
    jnz .clear_loop
    ret

times 510-($-$$) db 0
dw 0xaa55
