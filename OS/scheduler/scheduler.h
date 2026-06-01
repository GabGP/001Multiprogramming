#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../../lib/stdio.h"
#include "queue.h"

#ifdef PLATFORM_VERSATILEPB // ARM926EJ-S

    // VesatilePB base address
    #define MEM_ADDR 0x00000000

#elif defined(PLATFORM_BEAGLEBONE) // ARM Cortex-A8 (AM335x)

    // BeagleBone Black base address
    #define MEM_ADDR 0x82000000

#endif

// ============================================================================
// Process Control Block (PCB)
// ============================================================================

typedef enum
{
    PROCESS_NEW = 0,
    PROCESS_READY = 1,
    PROCESS_RUNNING = 2,
    PROCESS_WAITING = 3,
    PROCESS_SUSPENDED = 4,
    PROCESS_TERMINATED = 5
} ProcessState;

typedef enum
{
    NO_FAULT = 0,
    INVALID_MAPPING_FAULT = 1, // PAGE FAULT
    PRIVILEGE_VIOLATION_FAULT = 2,
    ALIGMENT_ERROR_FAULT = 3,
    PERMISSION_FAULT = 4,
    UNKNOWN_FAULT = 5
} FaultType;

typedef enum
{
    NORMAL_EXIT = 0,
    SYSCALL_EXIT = 1,
    FAULT_EXIT = 2
} TerminationReason;

typedef struct
{
    // Process context
    unsigned int regs[13];  // R0 - R12
    unsigned int sp;        // Stack pointer (R13)
    unsigned int lr;        // Link register (R14)
    unsigned int pc;        // Program counter (R15)
    unsigned int spsr;      // Saved Program Status Register

    // Process information
    unsigned int pid;       // Process ID
    unsigned int state;     // NEW, READY, RUNNING, WAITING, SUSPENDED, TERMINATED

    unsigned int syscall_id;            // SYS_YIELD, SYS_EXIT, SYS_WRITE
    unsigned int fault_type;            // NO_FAULT, INVALID_MAPPING_FAULT, PRIVILEGE_VIOLATION_FAULT, ALIGMENT_ERROR_FAULT, PERMISSION_FAULT, UNKNOWN_FAULT
    unsigned int termination_reason;    // NORMAL_EXIT, SYSCALL_EXIT, FAULT_EXIT
    int exit_code;                      // 0 >= normal exit, < 0 for error codes
} PCB;

// Number of user processes (OS, P1 and P2)
#define MAX_PROCESSES 3

extern PCB pcb[MAX_PROCESSES];

void pcb_init(unsigned int pid);
void setup_process_stack(unsigned int pid);
unsigned int get_process_stack_top(unsigned int pid);
void update_process_state(unsigned int pid, ProcessState new_state);

// ============================================================================
// Scheduler
// ============================================================================

extern Queue ready_queue;
extern unsigned int current_process;
extern unsigned int next_process;
extern unsigned int quantum;

void scheduler_init(void);
void schedule(void);
void schedule_yield(void);

void save_context(void);
void restore_context(void);

#endif // SCHEDULER_H