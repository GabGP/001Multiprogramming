#include "../../lib/stdio.h"
#include "../../lib/syscalls.h"

#ifdef PLATFORM_VERSATILEPB // ARM926EJ-S

    #define P2_ADDR 0x00200000

#elif defined(PLATFORM_BEAGLEBONE) // ARM Cortex-A8 (AM335x)
    
    #define P2_ADDR 0x82200000

#endif

#define TEST 1

int process_2(void) 
{
    if (TEST == 1) 
    {
        sys_write(1, "[C] Testing Alignment Fault\n", 28);
        
        // Read unaligned address in valid region
        volatile unsigned int *unaligned_ptr = (volatile unsigned int *)(P2_ADDR + 3);
        unsigned int value = *unaligned_ptr;
        (void)value;
    }
    else if (TEST == 2) 
    {
        sys_write(1, "[C] Testing Permission Fault\n", 32);
        
        // Read address without permissions
        volatile unsigned int *unallowed_ptr = (volatile unsigned int *)0x30000000;
        unsigned int value = *unallowed_ptr;
        (void)value;
    }
    else if (TEST == 3) 
    {
        sys_write(1, "[C] Testing Privilege Escalation\n", 33);
        
        // Attempt to access privileged system register from user mode
        unsigned int sctlr;
        asm volatile (
            "MRC p15, 0, %0, c1, c0, 0" // Read SCTLR
            : "=r" (sctlr)
        );
        (void)sctlr;
    }

    sys_write(1, "[C] ERROR: Fault was not caught!\n", 33);
    sys_exit(1);
    return 0; 
}
