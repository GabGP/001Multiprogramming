#ifndef KERNEL_H
#define KERNEL_H

#include "stdio.h"
#include "intc.h"
#include "timer.h"
#include "scheduler.h"
#include "process.h"
#include "mpu.h"

// ============================================================================
// Kernel Functions
// ============================================================================

#define IRQ_TIME 100 // How often the IRQ fires (ms)

void kernel_init(void);
extern void init_launch(void);

#endif // KERNEL_H
