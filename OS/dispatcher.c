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
        pcb[current_process].regs[0] = SUCCESS_RC;
        return_code = SUCCESS_RC;
        schedule_yield();
    }
    else if (id == SYS_EXIT)
    {
        update_process_state(current_process, PROCESS_TERMINATED);
        pcb[current_process].regs[0] = (int)arg1;
        pcb[current_process].exit_code = (int)arg1;
        return_code = (int)arg1;
        schedule();
    }
    else if (id == SYS_WRITE)
    {
        int fd = (int)arg1;
        char *buf = (char *)arg2;
        int len = (int)arg3;

        // Validate file descriptor and enforcing len cap
        if (fd != 1 || len < 0 || len > MAX_WRITE_LEN) 
        {
            pcb[current_process].regs[0] = INVALID_ARGUMENT_RC;
            return_code = INVALID_ARGUMENT_RC;
        }
        // Validate buf/len as a readable user range and validate user pointer
        else if (buf == 0 || 
                (unsigned int)buf < 0x10000 || // Block kernel access
                (unsigned int)(buf + len) < (unsigned int)buf || // Check for overflow
                (unsigned int)buf < (MEM_ADDR + current_process * 0x100000) || // Check bottom task boundary
                (unsigned int)(buf + len) > (MEM_ADDR + (current_process + 1) * 0x100000)) // Check top task boundary
        {
            pcb[current_process].regs[0] = INVALID_USR_PTR_RC;
            return_code = INVALID_USR_PTR_RC;
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                PRINT("%c", buf[i]);
            }
            pcb[current_process].regs[0] = len; // Return byte count
            return_code = len;
        }
    }
    else
    {   
        update_process_state(current_process, PROCESS_TERMINATED);        
        pcb[current_process].regs[0] = INVALID_SYSCALL_RC;
        return_code = INVALID_SYSCALL_RC;
        schedule();
    }

    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=syscall_return id=%d rc=%d\n", 
          current_process, id, return_code);
}

// Dispatcher for IRQs
void irq_dispatcher(void)
{
    PRINT("MODE_SWITCH USER_TO_KERNEL pid=%d reason=timer_irq\n", current_process);
    
    schedule();
    timer_irq_handler(); // Acknowledge the interrupt
    
    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=dispatch\n", current_process);
}

// Dispatcher for Aborts
void fault_dispatcher(unsigned int fault_type)
{
    PRINT("MODE_SWITCH USER_TO_KERNEL pid=%d reason=fault type=%d\n", current_process, fault_type);
    
    // Terminate faulting task and pick next runnable task
    update_process_state(current_process, PROCESS_TERMINATED);
    pcb[current_process].fault_type = fault_type;
    schedule(); 
    
    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=fault_recovery\n", current_process);
}