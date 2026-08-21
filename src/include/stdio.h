#ifndef STDIO_H
#define STDIO_H
 
#include "uart.h"
#include "stdlib.h"

#include <stdarg.h> // For dynamic argument handling
 
#define BUF_SIZE 256
 
// ============================================================================
// Standard I/O Functions
// ============================================================================
 
void PRINT(const char *fmt, ...);
void READ(const char *fmt, ...);
 
#endif