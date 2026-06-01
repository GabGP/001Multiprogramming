#include "../../lib/stdio.h"
#include "../../lib/syscalls.h"

int process_1(void) { 
    const char *msg = "[A] tick\n"; 

    for (int i = 0; i < 20; i++) {

        int32_t n = sys_write(1, msg, 9); 

        if (n < 0) { 
            /* optional */ 
        }

        // Small delay to prevent overwhelming UART
        for (volatile int i = 0; i < 95000000; i++)
            ;

        sys_yield(); 
    }

    sys_exit(0);

    return 0; 
} 