#ifndef LOGS_H
#define LOGS_H

#include "../lib/stdio.h"
#include "../OS/scheduler/scheduler.h"

// ============================================================================
// Debugging Functions
// ============================================================================

#define NUM_REGISTERS 18
unsigned int temp_regs[NUM_REGISTERS];

void log_pcb(void);
void log_registers(void);

#endif