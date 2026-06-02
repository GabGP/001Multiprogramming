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
        unsigned int val = *unaligned_ptr; 
        (void)val;
    }
    else if (TEST == 2) 
    {
        sys_write(1, "[C] Testing Sync External Abort\n", 32);
        
        // Read address in unmapped region
        volatile unsigned int *unmapped_ptr = (volatile unsigned int *)0x30000000;
        unsigned int val = *unmapped_ptr; 
        (void)val;
    }

    sys_write(1, "[C] ERROR: Fault was not caught!\n", 33);
    sys_exit(1);
    return 0; 
}
