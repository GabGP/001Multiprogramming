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
    FAULT_NONE = 0,
    FAULT_ALIGMENT_ERROR = 1, // ALIGNMENT FAULT
    FAULT_ACCESS_FLAG = 2,
    FAULT_INVALID_MAPPING = 3, // TRANSLATION FAULT
    FAULT_PRIV_VIOLATION = 4, // PRIVILEGE VIOLATION / DOMAIN FAULT
    FAULT_PERMISSION = 5,
    FAULT_SYNC_EXT_ABORT = 6, // SYNCHRONOUS EXTERNAL ABORT
    FAULT_UNKNOWN = -1,
    FAULT_UND_INST = -2 // UNDEFINED INSTRUCTION
} FaultType;

typedef enum
{
    EXIT_NORMAL = 0,
    EXIT_SYSCALL = 1,
    EXIT_FAULT = 2
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
    int fault_type;                     // FAULT_NONE, FAULT_INVALID_MAPPING, FAULT_PRIV_VIOLATION, FAULT_ALIGMENT_ERROR, FAULT_PERMISSION, FAULT_SYNC_EXT_ABORT, FAULT_UNKNOWN
    unsigned int termination_reason;    // EXIT_NORMAL, EXIT_SYSCALL, EXIT_FAULT
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

#define DEFAULT_QUANTUM 10

extern Queue ready_queue;
extern unsigned int current_process;
extern unsigned int next_process;
extern unsigned int quantum;

void scheduler_init(void);
void schedule(unsigned int is_yield);

#endif // SCHEDULER_H