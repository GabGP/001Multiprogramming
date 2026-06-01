#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../lib/stdio.h"
#include "../lib/syscalls.h"
#include "../drivers/timer.h"
#include "scheduler/scheduler.h"

// ============================================================================
// Dispatcher Functions
// ============================================================================

#define MAX_WRITE_LEN 256

// Return Codes
enum {
    SUCCESS_RC = 0,
    INVALID_SYSCALL_RC = -1,    // Invalid syscall ID
    INVALID_ARGUMENT_RC = -2,   // Invalid descriptor or argument
    INVALID_USR_PTR_RC = -3     // Invalid user pointer or protection violation
};

void syscall_dispatcher(void);
void irq_dispatcher(void);
void fault_dispatcher(unsigned int fault_type);

#endif