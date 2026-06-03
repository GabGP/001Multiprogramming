#include "dispatcher.h"

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

    if (id == SYS_YIELD)
    {
        pcb[current_process].syscall_id = SYS_YIELD;
        pcb[current_process].regs[0] = RC_SUCCESS;
        return_code = RC_SUCCESS;
        schedule(1);
    }
    else if (id == SYS_EXIT)
    {
        update_process_state(current_process, PROCESS_TERMINATED);
        pcb[current_process].syscall_id = SYS_EXIT;
        pcb[current_process].termination_reason = EXIT_SYSCALL;
        pcb[current_process].regs[0] = (int)arg1;
        pcb[current_process].exit_code = (int)arg1;
        return_code = (int)arg1;
        schedule(0);
    }
    else if (id == SYS_WRITE)
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
        else if (buf == 0 || 
                (unsigned int)buf < 0x10000 || // Block kernel access
                (unsigned int)(buf + len) < (unsigned int)buf || // Check for overflow
                (unsigned int)buf < (MEM_ADDR + current_process * 0x100000) || // Check bottom task boundary
                (unsigned int)(buf + len) > (MEM_ADDR + (current_process + 1) * 0x100000)) // Check top task boundary
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
    }
    else
    {   
        update_process_state(current_process, PROCESS_TERMINATED);
        pcb[current_process].syscall_id = id;
        pcb[current_process].termination_reason = EXIT_SYSCALL;
        pcb[current_process].exit_code = -1;     
        pcb[current_process].regs[0] = RC_INVALID_SYSCALL;
        return_code = RC_INVALID_SYSCALL;
        schedule(0);
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

    //  Decoding fault status (section/page)
    if (fault_status == 0x00) 
    {
        fault_type = FAULT_UND_INST;
    }
    else if (fault_status == 0x01) 
    {
        fault_type = FAULT_ALIGMENT_ERROR;
    }
    else if (fault_status == 0x03 || fault_status == 0x06) 
    {
        fault_type = FAULT_ACCESS_FLAG;
    }
    else if (fault_status == 0x05 || fault_status == 0x07) 
    {
        fault_type = FAULT_INVALID_MAPPING;
    }
    else if (fault_status == 0x08) 
    {
        fault_type = FAULT_SYNC_EXT_ABORT;
    }
    else if (fault_status == 0x09 || fault_status == 0x0B) 
    {
        fault_type = FAULT_PRIV_VIOLATION;
    }
    else if (fault_status == 0x0D || fault_status == 0x0F) 
    {
        fault_type = FAULT_PERMISSION; 
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
