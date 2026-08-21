#include "stdio.h"

// ============================================================================
// Standard I/O Functions
// ============================================================================

// Function to print formatted output based on format specifier
// (%d for integers, %c for characters, %s for strings, %x for hexadecimal)
void PRINT(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buf[BUF_SIZE];

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            if (*fmt == '\0')
            {
                uart_putc('%');
                break;
            }
            switch (*fmt)
            {
            case 'd':
            {
                int val = va_arg(args, int);
                itoa(val, buf);
                uart_puts(buf);
                break;
            }
            case 'c':
            {
                int val = va_arg(args, int);
                uart_putc((char)val);
                break;
            }
            case 's':
            {
                const char *s = va_arg(args, const char *);
                uart_puts(s);
                break;
            }
            case 'x':
            {
                int val = va_arg(args, int);
                itox(val, buf);
                uart_puts(buf);
                break;
            }
            case '%':
            {
                uart_putc('%');
                break;
            }
            default:
                /* Unknown specifier: print as-is */
                uart_putc('%');
                uart_putc(*fmt);
                break;
            }
        }
        else
        {
            uart_putc(*fmt);
        }
        fmt++;
    }

    va_end(args);
}

// Function to read input based on format specifiers
// (%d for integers, %s for strings)
void READ(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            switch (*fmt)
            {
            case 'd':
            {
                int *ptr = va_arg(args, int *);
                char buf[BUF_SIZE];
                uart_gets(buf, BUF_SIZE);
                *ptr = atoi(buf);
                break;
            }
            case 's':
            {
                char *ptr = va_arg(args, char *);
                uart_gets(ptr, BUF_SIZE);
                break;
            }
            default:
                /* Unknown specifier: ignore */
                break;
            }
        }
        fmt++;
    }

    va_end(args);
}