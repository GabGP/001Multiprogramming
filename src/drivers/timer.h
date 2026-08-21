#ifndef TIMER_H
#define TIMER_H

#include "stdio.h"

// Macro to access memory-mapped registers
#define REG(x) (*(volatile unsigned int * const)(x))

// ============================================================================
// Timer Register Definitions
// ============================================================================

#ifdef PLATFORM_VERSATILEPB // ARM926EJ-S

    #define TIMER_CLK_SPD 1000000 // 1 MHz

    // VersatilePB TIMER0 base address
    #define TIMER0_BASE 0x101E2000
    #define TIMER0_LOAD (TIMER0_BASE + 0x00) // Load Register
    #define TIMER0_CONTROL (TIMER0_BASE + 0x08) // Control Register
    #define TIMER0_INTCLR (TIMER0_BASE + 0x0C) // Interrupt Clear Register

    // VersatilePB Vector Interrupt Controller (VIC) base address
    #define VIC_BASE 0x10140000
    #define VIC_INTENABLE (VIC_BASE + 0x10) // Interrupt Enable Register
    #define VIC_INTSELECT (VIC_BASE + 0x0C) // Interrupt Select Register
    #define VIC_VECTADDR (VIC_BASE + 0x30)  // Vector Address Register

#elif defined(PLATFORM_BEAGLEBONE) // ARM Cortex-A8 (AM335x)

    #define TIMER_CLK_SPD 24000000 // 24 MHz
    #define MAX_COUNTER 0xFFFFFFFF

    // BeagleBone Black watchdog timer base address
    #define WDT1_BASE 0x44E35000
    #define WDT1_WWPS (WDT1_BASE + 0x34) // Watchdog Write Posted Status Register
    #define WDT1_WSPR (WDT1_BASE + 0x48) // Watchdog Start/Stop Register

    // BeagleBone Black DMTIMER2 base address
    #define DMTIMER2_BASE 0x48040000
    #define DTIMER2_TISR (DMTIMER2_BASE + 0x28) // Timer Interrupt Status Register (IRQSTATUS)
    #define DTIMER2_TIER (DMTIMER2_BASE + 0x2C) // Timer Interrupt Enable Register (IRQSTATUS_SET)
    #define DTIMER2_TCLR (DMTIMER2_BASE + 0x38) // Timer Control Register (TCLR)
    #define DTIMER2_TCRR (DMTIMER2_BASE + 0x3C) // Timer Counter Register (TCRR)
    #define DTIMER2_TLDR (DMTIMER2_BASE + 0x40) // Timer Load Register (TLDR)

    // BeagleBone Black Interrupt Controller (INTCPS) base address
    #define INTCPS_BASE 0x48200000
    #define INTC_MIR_CLEAR2 (INTCPS_BASE + 0xC8) // Interrupt Mask Clear Register 2
    #define INTC_CONTROL (INTCPS_BASE + 0x48)    // Interrupt Controller Control
    #define INTC_ILR68 (INTCPS_BASE + 0x110)     // Interrupt Line Register 68 (INTC_ILR4)

    // Clock Manager base address
    #define CM_PER_BASE 0x44E00000
    #define CM_PER_TIMER2_CLKCTRL (CM_PER_BASE + 0x80) // Timer2 Clock Control

#endif

// ============================================================================
// Watchdog Timer Functions
// ============================================================================

void watchdog_disable(void);

// ============================================================================
// Timer Functions
// ============================================================================

void timer_init(unsigned int time_ms);
void timer_irq_handler(void);

#endif