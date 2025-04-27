ENTRY(_start)

SECTIONS
{
    /* Kernel is loaded at 0x00007E00 */
    . = 0x7E00;

    _kernel_start = .;

    .text :
    {
        *(.text.kernel_entry)
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(.bss*)
        *(COMMON)
    }

    _kernel_end = .;
}
