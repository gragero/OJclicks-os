ENTRY(_start)

SECTIONS
{
    
    . = 0x1000;

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        bss_start = .; 
        *(.bss)
        *(COMMON)
        bss_end = .;   
    }

    kernel_end = .; 
}
