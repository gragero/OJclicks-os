ENTRY(_start)

SECTIONS
{
    /* لازم العنوان ده يطابق KERNEL_OFFSET في boot.asm بالضبط
     * لأن BIOS بيحمّل الكيرنل فعليًا في الميموري عند العنوان ده
     */
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
        bss_start = .; /* أول عنوان لقسم BSS - هنستخدمه عشان نصفّر المنطقة دي يدويًا */
        *(.bss)
        *(COMMON)
        bss_end = .;   /* آخر عنوان لقسم BSS */
    }

    kernel_end = .; /* عنوان نهاية الكيرنل في الذاكرة - هنستخدمه في الـ PMM لاحقًا */
}
