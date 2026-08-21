#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "stdio.h"
#include "syscalls.h"
#include "timer.h"
#include "uart.h"
#include "scheduler.h"

// ============================================================================
// Dispatcher Functions
// ============================================================================

#define MAX_WRITE_LEN 256

// Return Codes
typedef enum {
    RC_SUCCESS = 0,
    RC_INVALID_SYSCALL = -1,    // Invalid syscall ID
    RC_INVALID_ARGUMENT = -2,   // Invalid descriptor or argument
    RC_INVALID_USR_PTR = -3,    // Invalid user pointer or protection violation
    RC_INVALID_ORIGIN = -4      // Invalid syscall origin (triggered by kernel)
} ReturnCode;

void syscall_dispatcher(void);
void irq_dispatcher(void);
void fault_dispatcher(unsigned int fault_type);

extern void kernel_panic(void);

#endif // DISPATCHER_H
