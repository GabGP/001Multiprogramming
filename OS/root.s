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
    // Set stack pointer for IRQs
    msr CPSR, #0xD2 @ IRQ mode (0b10010) + IRQ/FIQ disabled
    ldr sp, =_stack_top

    // Set stack pointer for SWI/SVC mode
    msr CPSR, #0xD3 @ SVC mode (0b10011) + IRQ/FIQ disabled
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
    cmp r0, r1          @ While the current address is less than the end of .bss
    it lt
    strlt r2, [r0], #4  @ Clear current location and go to the next (increment by 4 bytes)
    blt clear_bss

    // Place a memory barrier
    dsb @ Data Synchronization Barrier
    isb @ Instruction Synchronization Barrier

    // Set up the ARM vector table via VBAR (Vector Base Address Register)
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0  @ Write the address of the vector table to VBAR

    // Jump to the C main function
    bl kernel_init

    // Load the dedicated stack for PID 0 before entering the OS process loop
    ldr r0, =pcb
    ldr sp, [r0, #52]

    // Start running the OS context as PID 0
    bl os_process

    // If the OS context ever returns, loop forever
hang:
    b hang

undefined_handler:
    bl log_registers
    bl log_pcb
    b hang

swi_handler:
    SAVE_CONTEXT 0
    bl syscall_dispatcher
    RESTORE_CONTEXT 0

prefetch_handler:
    SAVE_CONTEXT 4
    mov r0, #1          @ fault_type
    bl fault_dispatcher
    RESTORE_CONTEXT 4

data_handler:
    SAVE_CONTEXT 8
    mov r0, #2          @ fault_type
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
