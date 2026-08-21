#include "uart.h"

// ============================================================================
// UART Functions
// ============================================================================

// Function to send a single character via UART
void uart_putc(char c)
{
    // Wait until there is space in the FIFO
    while ((REG(UART0_FR) & UART0_FR_TXFE) == 0)
        ;
    REG(UART0_DR) = c;
}

// Function to receive a single character via UART
char uart_getc(void)
{
    // Wait until data is available
    while ((REG(UART0_FR) & UART0_FR_RXFE) != 0)
        ;
    return (char)(REG(UART0_DR) & 0xFF);
}

// Function to send a string via UART
void uart_puts(const char *s)
{
    while (*s)
    {
        uart_putc(*s++);
    }
}

// Function to receive a line of input via UART
void uart_gets(char *buffer, int max_length)
{
    int i = 0;
    char c;
    while (i < max_length - 1) // Leave space for null terminator
    {
        c = uart_getc();
        if (c == '\n' || c == '\r')
        {
            uart_putc('\n'); // Echo newline
            break;
        }
        uart_putc(c); // Echo character
        buffer[i++] = c;
    }
    buffer[i] = '\0'; // Null terminate the string
}
