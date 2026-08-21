#include "scheduler.h"

// ============================================================================
// Scheduler
// ============================================================================

unsigned int current_process = 0;
unsigned int quantum = DEFAULT_QUANTUM;

// Function to initialize the scheduler and ready queue
void scheduler_init(void)
{
    system_queue_init();
    init_queue(&ready_queue);
    quantum = DEFAULT_QUANTUM;
}

// Function to choose the next process to run (round-robin scheduler)
// - is_yield: 0, normal scheduling
// - is_yield: 1, voluntary yield
void schedule(unsigned int is_yield)
{
    if (quantum == 0 || pcb[current_process].state == PROCESS_TERMINATED || is_yield || current_process == 0)
    {
        if (is_yield)
            PRINT(".\n");
        else
            PRINT("...\n");

        if (!is_empty(&ready_queue))
        {
            // Put the current process back in line, if runnable. Except PID 0
            if (is_process_runnable(current_process) && current_process != 0)
            {
                update_process_state(current_process, PROCESS_READY);
                enqueue(&ready_queue, current_process);
            }

            // Get the next process
            current_process = dequeue(&ready_queue);
            update_process_state(current_process, PROCESS_RUNNING);
        }

        quantum = DEFAULT_QUANTUM;
    }
    else
    {
        quantum--;
    }
}
