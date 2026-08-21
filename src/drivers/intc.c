#include "intc.h"

// ============================================================================
// Interrupt Functions
// ============================================================================

void enable_irq(void)
{
    // Enable IRQs by clearing the I bit in CPSR
    asm volatile(
        "mrs r0, CPSR \n"
        "mov r1, #0xFFFFFF7F \n"
        "and r0, r0, r1 \n"
        "msr CPSR, r0 \n" ::: "r0", "r1", "memory");
}

void disable_irq(void)
{
    // Disable IRQs by setting the I bit in CPSR
    asm volatile(
        "mrs r0, CPSR \n"
        "mov r1, #0x80 \n"
        "orr r0, r0, r1 \n"
        "msr CPSR, r0 \n" ::: "r0", "r1", "memory");
}
