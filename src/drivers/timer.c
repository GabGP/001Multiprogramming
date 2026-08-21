#include "timer.h"

// ============================================================================
// Watchdog Timer Functions
// ============================================================================

// Function to disable the watchdog timer
void watchdog_disable(void)
{
#ifdef PLATFORM_BEAGLEBONE
    // Write 0xAAAA then 0x5555 to WDT1_WSPR, waiting for WDT1_WWPS between writes
    REG(WDT1_WSPR) = 0xAAAA;
    while (REG(WDT1_WWPS) != 0)
        ;

    REG(WDT1_WSPR) = 0x5555;
    while (REG(WDT1_WWPS) != 0)
        ;
#endif
}

// ============================================================================
// Timer Functions
// ============================================================================

// Function to initialize the timer to generate an interrupt every ? ms
void timer_init(unsigned int time_ms)
{
#ifdef PLATFORM_VERSATILEPB
    // Setting the load value to ? millisecond(s)
    // clk: 1 MHz, t: ?, f: 1/t Hz, N = (1MHz/1000)*t_ms
    REG(TIMER0_LOAD) = (unsigned int)(TIMER_CLK_SPD / 1000) * time_ms;

    // Enable timer, periodic mode, IRQ, 32-bit counter
    REG(TIMER0_CONTROL) = 0xE2;

    // IRQ interrupt
    REG(VIC_INTSELECT) &= ~(1<<4);

    // Enable interrupt
    REG(VIC_INTENABLE) |= (1<<4);   

#elif defined(PLATFORM_BEAGLEBONE)
    // Enabling the timer clock
    REG(CM_PER_TIMER2_CLKCTRL) = 0x2;

    // Unmasking IRQ 68
    // Each register of INTC_MIR_CLEAR2 belongs to an IRQ
    // There are 3 INTC_MIR, so 96 IRQ
    // bit 0 of INTC_MIR_CLEAR2 -> IRQ 64,
    // bit 1 of INTC_MIR_CLEAR2 -> IRQ 65,
    // bit 2 of INTC_MIR_CLEAR2 -> IRQ 66,
    // bit 3 of INTC_MIR_CLEAR2 -> IRQ 67,
    // bit 4 of INTC_MIR_CLEAR2 -> IRQ 68,
    // etc ...
    REG(INTC_MIR_CLEAR2) = 0x10;

    // Setting interrupt priority to 0x0
    REG(INTC_ILR68) = 0x0;

    // Stopping the timer
    REG(DTIMER2_TCLR) = 0x0;

    // Clearing pending interrupts
    REG(DTIMER2_TISR) = 0x7;

    // Setting the load value to ? millisecond(s)
    // clk: 24 MHz, t: ? ms, f: 1/t Hz, N = (24MHz/1000)*time_ms
    // max_counter: 0xFFFFFFFF, timer_counter = max_counter - N
    REG(DTIMER2_TLDR) = (unsigned int)MAX_COUNTER - (((unsigned int)TIMER_CLK_SPD / 1000) * time_ms);

    // Setting the counter to the same value
    REG(DTIMER2_TCRR) = REG(DTIMER2_TLDR);

    // Enabling overflow interrupt
    REG(DTIMER2_TIER) = 0x2;

    // Starting the timer in auto-reload mode
    REG(DTIMER2_TCLR) = 0x3;
#endif
}

// Function to handle timer interrupts
void timer_irq_handler(void)
{
#ifdef PLATFORM_VERSATILEPB
    // Clearing the timer interrupt
    REG(TIMER0_INTCLR) = 0x1;

    // Acknowledging the interrupt to the controller
    REG(VIC_VECTADDR) = 0x0;

#elif defined(PLATFORM_BEAGLEBONE)
    // Clearing the timer interrupt flag
    REG(DTIMER2_TISR) = 0x2;

    // Acknowledging the interrupt to the controller
    REG(INTC_CONTROL) = 0x1;
#endif
}
