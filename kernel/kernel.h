#ifndef KERNEL_H
#define KERNEL_H

#include "../lib/stdio.h"
#include "../drivers/intc.h"
#include "../drivers/timer.h"
#include "scheduler/scheduler.h"
#include "./mpu.h"

// ============================================================================
// Kernel Functions
// ============================================================================

#define IRQ_TIME 100 // How often the IRQ fires (ms)

extern unsigned int process_count;

void create_process(unsigned int pid);
void kernel_init(void);
extern void init_launch(void);

#endif // KERNEL_H
