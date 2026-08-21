#include "dispatcher.h"

// ============================================================================
// Helper Functions
// ============================================================================

// Validate that a user pointer and buffer length falls within the process's allocated memory range
static int is_valid_user_ptr(const char *buf, int len, int pid)
{
    if (buf == 0)
        return 0;
    if ((unsigned int)buf < 0x10000) // Block kernel access
        return 0;
    if ((unsigned int)(buf + len) < (unsigned int)buf) // Check for overflow
        return 0;
    if ((unsigned int)buf < (MEM_ADDR + pid * 0x100000)) // Check bottom task boundary
        return 0;
    if ((unsigned int)(buf + len) > (MEM_ADDR + (pid + 1) * 0x100000)) // Check top task boundary
        return 0;

    return 1;
}

// ============================================================================
// Dispatcher Functions
// ============================================================================

// Dispatcher for Syscalls
void syscall_dispatcher(void)
{
    int return_code = 0;

    // Get saved syscall ID and arguments
    unsigned int id = pcb[current_process].regs[0];
    unsigned int arg1 = pcb[current_process].regs[1];
    unsigned int arg2 = pcb[current_process].regs[2];
    unsigned int arg3 = pcb[current_process].regs[3];

    PRINT("MODE_SWITCH USER_TO_KERNEL pid=%d reason=syscall id=%d\n", current_process, id);

    // Check if the syscall originated from user mode
    if ((pcb[current_process].spsr & 0x1F) != 0x10)
    {
        update_process_state(current_process, PROCESS_TERMINATED);
        pcb[current_process].syscall_id = id;
        pcb[current_process].termination_reason = EXIT_SYSCALL;
        pcb[current_process].exit_code = -1;
        pcb[current_process].regs[0] = RC_INVALID_ORIGIN;
        kernel_panic();
        return;
    }

    switch (id)
    {
    case SYS_YIELD:
        pcb[current_process].syscall_id = SYS_YIELD;
        pcb[current_process].regs[0] = RC_SUCCESS;
        return_code = RC_SUCCESS;
        schedule(1);
        break;

    case SYS_EXIT:
        update_process_state(current_process, PROCESS_TERMINATED);
        pcb[current_process].syscall_id = SYS_EXIT;
        pcb[current_process].termination_reason = EXIT_SYSCALL;
        pcb[current_process].regs[0] = (int)arg1;
        pcb[current_process].exit_code = (int)arg1;
        return_code = (int)arg1;
        schedule(0);
        break;

    case SYS_WRITE:
    {
        int fd = (int)arg1;
        char *buf = (char *)arg2;
        int len = (int)arg3;

        pcb[current_process].syscall_id = SYS_WRITE;

        // Validate file descriptor and enforcing len cap
        if (fd != 1 || len < 0 || len > MAX_WRITE_LEN)
        {
            pcb[current_process].regs[0] = RC_INVALID_ARGUMENT;
            return_code = RC_INVALID_ARGUMENT;
        }
        // Validate buf/len as a readable user range and validate user pointer
        else if (!is_valid_user_ptr(buf, len, current_process))
        {
            pcb[current_process].regs[0] = RC_INVALID_USR_PTR;
            return_code = RC_INVALID_USR_PTR;
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                uart_putc(buf[i]);
            }
            pcb[current_process].regs[0] = len; // Return byte count
            return_code = len;
        }
        break;
    }

    default:
        update_process_state(current_process, PROCESS_TERMINATED);
        pcb[current_process].syscall_id = id;
        pcb[current_process].termination_reason = EXIT_SYSCALL;
        pcb[current_process].exit_code = -1;
        pcb[current_process].regs[0] = RC_INVALID_SYSCALL;
        return_code = RC_INVALID_SYSCALL;
        schedule(0);
        break;
    }

    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=syscall_return id=%d rc=%d\n",
          current_process, id, return_code);
}

// Dispatcher for IRQs
void irq_dispatcher(void)
{
    PRINT("MODE_SWITCH USER_TO_KERNEL pid=%d reason=timer_irq\n", current_process);

    schedule(0);
    timer_irq_handler(); // Acknowledge the interrupt

    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=dispatch\n", current_process);
}

// Dispatcher for Aborts
void fault_dispatcher(unsigned int fault_status_reg)
{
    // Get FS from IFSR/DFSR[10,3:0]
    unsigned int fault_status = ((fault_status_reg >> 6) & 0x10) | (fault_status_reg & 0x0F);
    int fault_type = FAULT_UNKNOWN;

    // Decoding fault status (section/page)
    switch (fault_status)
    {
    case 0x00:
        fault_type = FAULT_UND_INST;
        break;
    case 0x01:
        fault_type = FAULT_ALIGMENT_ERROR;
        break;
    case 0x02:
        fault_type = FAULT_DEBUG_EVENT;
        break;
    case 0x03:
    case 0x06:
        fault_type = FAULT_ACCESS_FLAG;
        break;
    case 0x04:
        fault_type = FAULT_INST_CACHE_MAINT;
        break;
    case 0x05:
    case 0x07:
        fault_type = FAULT_INVALID_MAPPING;
        break;
    case 0x08:
        fault_type = FAULT_SYNC_EXT_ABORT;
        break;
    case 0x09:
    case 0x0B:
        fault_type = FAULT_PRIV_VIOLATION;
        break;
    case 0x0C:
    case 0x0E:
        fault_type = FAULT_TTB_WALK_SEA;
        break;
    case 0x0D:
    case 0x0F:
        fault_type = FAULT_PERMISSION;
        break;
    case 0x14:
        fault_type = FAULT_IMP_DEF_LD;
        break;
    case 0x16:
        fault_type = FAULT_ASYNC_EXT_ABORT;
        break;
    case 0x18:
        fault_type = FAULT_MEM_ACCESS_APE;
        break;
    case 0x19:
        fault_type = FAULT_MEM_ACCESS_SPE;
        break;
    case 0x1A:
        fault_type = FAULT_IMP_DEF_CA;
        break;
    case 0x1C:
    case 0x1E:
        fault_type = FAULT_TTB_WALK_SPE;
        break;
    default:
        fault_type = FAULT_UNKNOWN;
        break;
    }

    PRINT("MODE_SWITCH USER_TO_KERNEL pid=%d reason=fault type=%d\n", current_process, fault_type);

    // Terminate faulting task and pick next runnable task
    update_process_state(current_process, PROCESS_TERMINATED);
    pcb[current_process].termination_reason = EXIT_FAULT;
    pcb[current_process].exit_code = -1;
    pcb[current_process].fault_type = fault_type;
    schedule(0);

    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=fault_recovery\n", current_process);
}
