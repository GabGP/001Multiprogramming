#ifndef UART_H
#define UART_H

// Macro to access memory-mapped registers
#define REG(x) (*(volatile unsigned int * const)(x))

// ============================================================================
// UART Register Definitions
// ============================================================================

#ifdef PLATFORM_VERSATILEPB

    #define UART0_BASE      0x101F1000
    #define UART0_DR        (UART0_BASE + 0x00) // Data Register
    #define UART0_FR        (UART0_BASE + 0x18) // Flag Register
    #define UART0_FR_TXFE   (1 << 7)            // Transmit FIFO Empty
    #define UART0_FR_RXFE   (1 << 4)            // Receive FIFO Empty

#elif defined(PLATFORM_BEAGLEBONE)

    #define UART0_BASE      0x44E09000
    #define UART0_DR        (UART0_BASE + 0x00) // Data Register (THR/RHR)
    #define UART0_FR        (UART0_BASE + 0x14) // Flag Register (UART LSR)
    #define UART0_FR_TXFE   (1 << 5)            // Transmit FIFO Empty (LSR_TXFIFOE)
    #define UART0_FR_RXFE   (1 << 4)            // Receive FIFO Empty (LSR_RXBI)
    
#endif

// ============================================================================
// UART Functions
// ============================================================================

void uart_putc(char c);
char uart_getc(void);
void uart_puts(const char *s);
void uart_gets(char *buffer, int max_length);

#endif
