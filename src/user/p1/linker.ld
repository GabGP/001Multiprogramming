ENTRY(_start)

MEMORY
{
    ram : ORIGIN = P1_ADDR, LENGTH = 64K
}

PHDRS
{
    text PT_LOAD FLAGS(5); /* r-x */
    data PT_LOAD FLAGS(6); /* rw- */
}

SECTIONS
{
    . = ORIGIN(ram);

    .text :
    {
        *(.text*)
        *(.rodata*)
    } > ram :text

    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
    } > ram :data

    .bss (NOLOAD) :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > ram :data

    _stack_bottom = __bss_end__;
    _stack_top = ORIGIN(ram) + LENGTH(ram);
}
