#include "../lib/stdio.h"
#include "../lib/syscalls.h"

// ============================================================================
// OS Functions
// ============================================================================

// Function of the OS process
void os_process(void)
{
    char c = 'A';
    char msg[] = "----From OS: X\n";

    while (1)
    {
        msg[13] = c;
        sys_write(1, msg, 15);

        c++;
        if (c > 'Z')
        {
            c = 'A';
        }

        #ifdef PLATFORM_VERSATILEPB // ARM926EJ-S

            // Small delay to prevent overwhelming UART
            for (volatile int i = 0; i < 95000000; i++)
                ;

        #elif defined(PLATFORM_BEAGLEBONE) // ARM Cortex-A8 (AM335x)
            
            // Small delay to prevent overwhelming UART
            for (volatile int i = 0; i < 2000000; i++)
                ;

        #endif

    }
}
