#include "process.h"

// ============================================================================
// Process Control Block (PCB)
// ============================================================================

PCB pcb[MAX_PROCESSES];
Queue ready_queue;

// Function to Initialize PCBs
void pcb_init(unsigned int pid)
{
    unsigned int stack_top = get_process_stack_top(pid);

    pcb[pid].pid = pid;
    pcb[pid].pc = MEM_ADDR + pid * 0x100000; // Entry point
    pcb[pid].sp = stack_top;
    pcb[pid].lr = MEM_ADDR + pid * 0x100000; // Entry point
    pcb[pid].spsr = 0x10;                    // User mode, IRQs enabled

    for (int i = 0; i < 13; i++)
        pcb[pid].regs[i] = 0x0;

    pcb[pid].syscall_id = 0;
    pcb[pid].fault_type = FAULT_NONE;
    pcb[pid].termination_reason = EXIT_NORMAL;
    pcb[pid].exit_code = 0;

    update_process_state(pid, PROCESS_NEW);

    // Memory barrier to ensure PCB writes are visible before any context switch
    asm volatile("dsb" ::: "memory");
}

// Function to setup the stack frame of a process
void setup_process_stack(unsigned int pid)
{
    unsigned int stack_top = get_process_stack_top(pid);

    // Building a saved context at the top of the region:
    // Set LR to the process entry point
    pcb[pid].lr = MEM_ADDR + pid * 0x100000;

    // Set the process SP directly to the stack top
    pcb[pid].sp = stack_top;

    // Set the PC to the process entry point
    pcb[pid].pc = MEM_ADDR + pid * 0x100000;
}

// Function to get the top of the process stack
unsigned int get_process_stack_top(unsigned int pid)
{
    unsigned int stack_top = MEM_ADDR + pid * 0x100000 + 0x10000;

    // PID 0 runs inside the OS image, so keep its process stack below the
    // exception stacks used by IRQ/SVC handlers at the top of the OS region.
    if (pid == 0)
    {
        stack_top -= 0x2000;
    }

    return stack_top;
}

// Function to create a process
void create_process(unsigned int pid)
{
    pcb_init(pid);
    setup_process_stack(pid);
    update_process_state(pid, PROCESS_READY);
}

// Function to update process state (NEW, READY, RUNNING, WAITING, SUSPENDED, TERMINATED)
void update_process_state(unsigned int pid, ProcessState new_state)
{
    if (pcb[pid].state == PROCESS_NEW && new_state == PROCESS_READY && pid != 0)
    {
        enqueue(&ready_queue, pid);
    }
    pcb[pid].state = new_state;
}

// Function to check if a process is runnable (READY, WAITING, RUNNING)
int is_process_runnable(unsigned int pid)
{
    return pcb[pid].state == PROCESS_READY ||
           pcb[pid].state == PROCESS_WAITING ||
           pcb[pid].state == PROCESS_RUNNING;
}
