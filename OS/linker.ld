ENTRY(_start)

MEMORY
{
    ram : ORIGIN = MEM_ADDR, LENGTH = 64K
}

SECTIONS
{
    . = ORIGIN(ram);

    .text :
    {
        *(.text*)
        *(.rodata*)
    } > ram

    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
    } > ram

    .bss :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > ram

    _stack_bottom = __bss_end__;
    _stack_top = ORIGIN(ram) + LENGTH(ram);

    /* PROCESS IMAGES WITH LMA COMPRESSION */

    .p1_image P1_ADDR : AT(ALIGN(LOADADDR(.bss) + SIZEOF(.bss), 4))
    {
        . = ALIGN(4);
        __p1_vma_start = .;
        *(.p1_image)
        . = ALIGN(4);
        __p1_vma_end = .;
    }
    __p1_lma_start = LOADADDR(.p1_image);

    .p2_image P2_ADDR : AT(ALIGN(LOADADDR(.p1_image) + SIZEOF(.p1_image), 4))
    {
        . = ALIGN(4);
        __p2_vma_start = .;
        *(.p2_image)
        . = ALIGN(4);
        __p2_vma_end = .;
    }
    __p2_lma_start = LOADADDR(.p2_image);
}
