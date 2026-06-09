#include "kernel.h"

// ============================================================================
// Kernel Functions
// ============================================================================

// Function to initialize the OS and create processes
static void os_init(void)
{
    unsigned int process_count = 1;

    PRINT("Starting OS ... ");
    scheduler_init();
    create_process(process_count++); // OS process
    PRINT("OK\n");

    PRINT("Creating user processes ... ");
    create_process(process_count++); // User process 1
    create_process(process_count++); // User process 2
    PRINT("OK\n\n");

    schedule(0);
    PRINT("MODE_SWITCH KERNEL_TO_USER pid=%d reason=initial_launch\n", current_process);
    init_launch();
}

// Function to initialize the kernel
void kernel_init(void)
{
    // Welcome message
    PRINT("\n=== 0001Multiprogramming ===\n");
    PRINT(" - Carlos Alvarez - 23004004\n");
    PRINT(" - Gabriel Garcia - 17001171\n");

    PRINT("\nStarting Kernel ...\n\n");

    // Initialize the MPU to set up memory regions and permissions
    PRINT("Initializing MPU ... ");
    mpu_init();
    PRINT("OK\n");

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
