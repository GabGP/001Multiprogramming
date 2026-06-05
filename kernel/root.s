.section .text
.syntax unified
.code 32
.globl _start

_start:
b reset_handler

.macro SAVE_CONTEXT pc_offset
    stmfd sp!, {r0-r12}

    ldr r0, =pcb
    ldr r1, =current_process
    ldr r2, [r1]
    mov r3, #92     @ sizeof(PCB) = 23 * 4
    mul r4, r3, r2
    add r5, r0, r4

    // Disable IRQs before switching modes
    mrs r6, CPSR
    orr r7, r6, #0x80
    msr CPSR, r7

    // Read the interrupted process SP/LR from System mode
    mrs r6, CPSR
    orr r7, r6, #0x1F
    msr CPSR, r7
    mov r8, sp
    mov r9, lr
    msr CPSR, r6

    // Copy R0-R12 from the exception stack into the PCB
    ldmfd sp!, {r0-r4}
    stmia r5!, {r0-r4}
    ldmfd sp!, {r0-r4}
    stmia r5!, {r0-r4}
    ldmfd sp!, {r0-r2}
    stmia r5!, {r0-r2}
    sub r5, r5, #52

    str r8, [r5, #52]
    str r9, [r5, #56]
    sub r6, lr, #\pc_offset
    str r6, [r5, #60]
    mrs r7, SPSR
    str r7, [r5, #64]
    dsb
.endm

.macro RESTORE_CONTEXT return_offset
    ldr r0, =pcb
    ldr r1, =current_process
    ldr r2, [r1]
    mov r3, #92     @ sizeof(PCB) = 23 * 4
    mul r4, r3, r2
    add r5, r0, r4

    ldr r6, [r5, #64]
    msr SPSR, r6
    ldr r7, [r5, #60]
    add lr, r7, #\return_offset

    // Disable IRQs before switching modes
    mrs r6, CPSR
    orr r7, r6, #0x80
    msr CPSR, r7

    // Restore the process SP/LR in System mode
    mrs r6, CPSR
    orr r7, r6, #0x1F
    msr CPSR, r7
    ldr sp, [r5, #52]
    ldr lr, [r5, #56]
    msr CPSR, r6

    ldm r5, {r0-r12}
    subs pc, lr, #\return_offset
.endm

// ARM Vector Table
.align 5                 @ Align to 32 bits (2^5)
vector_table:
    b reset_handler      @ 0x00: Reset
    b undefined_handler  @ 0x04: Undefined Instruction
    b swi_handler        @ 0x08: Software Interrupt (SWI)
    b prefetch_handler   @ 0x0C: Prefetch Abort
    b data_handler       @ 0x10: Data Abort
    b .                  @ 0x14: Reserved
    b irq_handler        @ 0x18: IRQ (Interrupt Request)
    b fiq_handler        @ 0x1C: FIQ (Fast Interrupt Request)

reset_handler:
    // Set stack pointer for FIQs
    msr CPSR, #0xD1 @ FIQ mode (0b10001) + IRQ/FIQ disabled
    ldr sp, =_stack_top

    // Set stack pointer for IRQs
    msr CPSR, #0xD2 @ IRQ mode (0b10010) + IRQ/FIQ disabled
    ldr sp, =_stack_top

    // Set stack pointer for SWI/SVC mode
    msr CPSR, #0xD3 @ SVC mode (0b10011) + IRQ/FIQ disabled
    ldr sp, =_stack_top

    // Set stack pointer for ABT mode
    msr CPSR, #0xD7 @ ABT mode (0b10111) + IRQ/FIQ disabled
    ldr sp, =_stack_top

    // Set stack pointer for UND mode
    msr CPSR, #0xDB @ UND mode (0b11011) + IRQ/FIQ disabled
    ldr sp, =_stack_top

    // Set CPU to System mode
    msr CPSR, #0xDF @ SYS mode (0b11111) + IRQ/FIQ disabled

    // Set the initial stack pointer for the OS
    ldr sp, =_stack_top

    // Clear the .bss section
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
clear_bss:
    cmp r0, r1          @ While the current address has not reached the end of .bss
    it lt
    strlt r2, [r0], #4  @ Clear current location and go to the next (increment by 4 bytes)
    blt clear_bss

    // Relocate Process OS (LMA to VMA)
    ldr r0, =__os_lma_start
    ldr r1, =__os_vma_start
    ldr r2, =__os_vma_end
relocate_os:
    cmp r1, r2
    bge os_relocation_done
    ldr r3, [r0], #4
    str r3, [r1], #4
    b relocate_os

os_relocation_done:
    // Relocate Process 1 (LMA to VMA)
    ldr r0, =__p1_lma_start
    ldr r1, =__p1_vma_start
    ldr r2, =__p1_vma_end
relocate_p1:
    cmp r1, r2
    bge p1_relocation_done
    ldr r3, [r0], #4
    str r3, [r1], #4
    b relocate_p1

p1_relocation_done:
    // Relocate Process 2 (LMA to VMA)
    ldr r0, =__p2_lma_start
    ldr r1, =__p2_vma_start
    ldr r2, =__p2_vma_end
relocate_p2:
    cmp r1, r2
    bge p2_relocation_done
    ldr r3, [r0], #4
    str r3, [r1], #4
    b relocate_p2

p2_relocation_done:
    // Place a memory barrier
    dsb @ Data Synchronization Barrier
    isb @ Instruction Synchronization Barrier

    // Set up the ARM vector table via VBAR (Vector Base Address Register)
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0  @ Write the address of the vector table to VBAR

    // Jump to the C main function
    bl kernel_init

    // If the kernel ever returns, loop forever
hang:
    b hang

.globl enable_mmu
enable_mmu:
    // Indicate the base address of the page table to the MMU
    ldr r1, =page_table
    mcr p15, 0, r1, c2, c0, 0   @ Write to TTBR0 (Translation Table Base Register)

    // Set domain 0 to client mode
    mov r0, #0x1
    mcr p15, 0, r0, c3, c0, 0   @ Write to DACR (Domain Access Control Register)
    
    // Set up SCTLR (System Control Register) to enable the MMU and Strict Alignment
    mrc p15, 0, r2, c1, c0, 0
    orr r2, r2, #0x1            @ Enable MMU (M bit)
    orr r2, r2, #0x2            @ Enable Strict Alignment (A bit)
    mcr p15, 0, r2, c1, c0, 0

    dsb @ Data Synchronization Barrier
    isb @ Instruction Synchronization Barrier
    bx lr

undefined_handler:
    SAVE_CONTEXT 4
    mov r0, #0
    bl fault_dispatcher
    RESTORE_CONTEXT 4

swi_handler:
    SAVE_CONTEXT 0
    bl syscall_dispatcher
    RESTORE_CONTEXT 0

prefetch_handler:
    SAVE_CONTEXT 4
    mrc p15, 0, r0, c5, c0, 1   @ IFSR
    bl fault_dispatcher
    RESTORE_CONTEXT 4

data_handler:
    SAVE_CONTEXT 8
    mrc p15, 0, r0, c5, c0, 0   @ DFSR
    bl fault_dispatcher
    RESTORE_CONTEXT 8

irq_handler:
    SAVE_CONTEXT 4
    bl irq_dispatcher
    RESTORE_CONTEXT 4

fiq_handler:
    bl log_registers
    bl log_pcb
    b hang

// Reserve a contiguous region for the process stack
.section .bss
.align 4            @ Align to 16 bits (2^4)
_stack_bottom:
    .skip 0x2000    @ 8KB stack space
_stack_top:
