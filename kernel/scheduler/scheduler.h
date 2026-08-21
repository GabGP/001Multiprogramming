#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../../lib/stdio.h"
#include "../process.h"
#include "queue.h"

// ============================================================================
// Scheduler
// ============================================================================

#define DEFAULT_QUANTUM 10

extern unsigned int current_process;
extern unsigned int quantum;

void scheduler_init(void);
void schedule(unsigned int is_yield);

#endif // SCHEDULER_H