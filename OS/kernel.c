#include "kernel.h"

// ============================================================================
// Kernel Functions
// ============================================================================

// Function to create a process
void create_process(unsigned int pid)
{
    pcb_init(pid);
    setup_process_stack(pid);
    update_process_state(pid, PROCESS_READY);
}

// ============================================================================
// Main Functions
// ============================================================================

unsigned int process_count = 0;

// Function to initialize the OS and create processes
void os_init(void)
{
    PRINT("Starting OS ... ");
    scheduler_init();
    create_process(process_count++); // OS process
    PRINT("OK\n");

    PRINT("Creating user processes ... ");
    create_process(process_count++); // User process 1
    create_process(process_count++); // User process 2
    PRINT("OK\n\n");

    current_process = 0;
    update_process_state(current_process, PROCESS_RUNNING);
    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=initial_launch\n", current_process);
}

// Function to initialize the kernel
void kernel_init(void)
{
    // Welcome message
    PRINT("\n=== 0001Multiprogramming ===\n");
    PRINT(" - Carlos Alvarez - 23004004\n");
    PRINT(" - Gabriel Garcia - 17001171\n");

    PRINT("\nStarting Kernel ...\n\n");

    // Disable the watchdog timer to prevent resets
    PRINT("Disabling watchdog ... ");
    watchdog_disable();
    PRINT("OK\n");

    PRINT("Initializing timer ... ");
    timer_init(IRQ_TIME);
    PRINT("OK\n");

    PRINT("Enabling interrupts ... ");
    enable_irq();
    PRINT("OK\n");

    PRINT("\nKernel started successfully.\n\n");

    // Initialize the OS and create processes
    os_init();
}

// Function of the OS process
void os_process(void)
{
    char c = 'A';

    while (1)
    {
        PRINT("----From OS: %c\n", c);

        c++;
        if (c > 'Z')
        {
            c = 'A';
        }

        sys_yield();

    }
}
