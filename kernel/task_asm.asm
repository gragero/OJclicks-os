[bits 32]

global task_switch
global task_switch_exit

task_switch:
    mov eax, [esp+4]
    mov edx, [esp+8]

    push ebx
    push esi
    push edi
    push ebp

    mov [eax], esp

    mov esp, edx

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret

task_switch_exit:
    mov edx, [esp+4]
    mov esp, edx

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret
