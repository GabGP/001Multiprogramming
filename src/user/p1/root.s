.section .text
.global _start

_start:
b reset

reset:
ldr sp, =_stack_top   @ Set up stack pointer (within RAM)
bl process_1          @ Call the main function
loop:
b loop                @ Infinite loop to prevent exit

// Stack space allocation
.section .bss
.align 4
_stack_bottom:
    .skip 0x2000  @ 8KB stack space
_stack_top:
