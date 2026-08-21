#include "logs.h"

// ============================================================================
// Debugging Functions
// ============================================================================

// Function to log the current pcb information
void log_pcb(void)
{
    const char *state_names[] = {"NEW", "READY", "RUNNING", "WAITING", "SUSPENDED", "TERMINATED"};

    PRINT("\n=== PCB of PID %d ===\n", pcb[current_process].pid);
    PRINT("State: %s, PC: 0x%x, SP: 0x%x\n", state_names[pcb[current_process].state], pcb[current_process].pc, pcb[current_process].sp);

    PRINT("R%d: 0x%x\n", 0, pcb[current_process].regs[0]);
    PRINT("R%d: 0x%x\n", 1, pcb[current_process].regs[1]);
    PRINT("R%d: 0x%x\n", 2, pcb[current_process].regs[2]);
    PRINT("R%d: 0x%x\n", 3, pcb[current_process].regs[3]);
    PRINT("R%d: 0x%x\n", 4, pcb[current_process].regs[4]);
    PRINT("R%d: 0x%x\n", 5, pcb[current_process].regs[5]);
    PRINT("R%d: 0x%x\n", 6, pcb[current_process].regs[6]);
    PRINT("R%d: 0x%x\n", 7, pcb[current_process].regs[7]);
    PRINT("R%d: 0x%x\n", 8, pcb[current_process].regs[8]);
    PRINT("R%d: 0x%x\n", 9, pcb[current_process].regs[9]);
    PRINT("R%d: 0x%x\n", 10, pcb[current_process].regs[10]);
    PRINT("R%d: 0x%x\n", 11, pcb[current_process].regs[11]);
    PRINT("R%d: 0x%x\n", 12, pcb[current_process].regs[12]);

    PRINT("LR: 0x%x, SPSR: 0x%x\n", pcb[current_process].lr, pcb[current_process].spsr);

    PRINT("Syscall ID: %d, Fault Type: %d, Termination Reason: %d, Exit Code: %d\n",
          pcb[current_process].syscall_id,
          pcb[current_process].fault_type,
          pcb[current_process].termination_reason,
          pcb[current_process].exit_code
        );
}

// Function to log the current registers
void log_registers(void)
{
    asm volatile(
        "stmia %0, {r0-r12} \n"
        "str sp, [%0, #52] \n" // Save SP (R13)
        "str lr, [%0, #56] \n" // Save LR (R14)
        "str pc, [%0, #60] \n" // Save PC (R15)
        "mrs r1, SPSR \n"
        "str r1, [%0, #64] \n" // Save SPSR
        "mrs r1, CPSR \n"
        "str r1, [%0, #68] \n" // Save CPSR
        :
        : "r"(temp_regs)
        : "r0", "r1", "memory");

    PRINT("\n=== Current Registers ===\n");

    PRINT("R%d: 0x%x\n", 0, temp_regs[0]);
    PRINT("R%d: 0x%x\n", 1, temp_regs[1]);
    PRINT("R%d: 0x%x\n", 2, temp_regs[2]);
    PRINT("R%d: 0x%x\n", 3, temp_regs[3]);
    PRINT("R%d: 0x%x\n", 4, temp_regs[4]);
    PRINT("R%d: 0x%x\n", 5, temp_regs[5]);
    PRINT("R%d: 0x%x\n", 6, temp_regs[6]);
    PRINT("R%d: 0x%x\n", 7, temp_regs[7]);
    PRINT("R%d: 0x%x\n", 8, temp_regs[8]);
    PRINT("R%d: 0x%x\n", 9, temp_regs[9]);
    PRINT("R%d: 0x%x\n", 10, temp_regs[10]);
    PRINT("R%d: 0x%x\n", 11, temp_regs[11]);
    PRINT("R%d: 0x%x\n", 12, temp_regs[12]);

    PRINT("SP: 0x%x, LR: 0x%x, PC: 0x%x\n", temp_regs[13], temp_regs[14], temp_regs[15]);
    PRINT("SPSR: 0x%x, CPSR: 0x%x\n\n", temp_regs[16], temp_regs[17]);
}