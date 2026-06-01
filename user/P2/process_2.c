#include "../../lib/stdio.h"
#include "../../lib/syscalls.h"

int process_2(void)
{
    int32_t r1 = sys_write(9, "X\n", 2); 
    int32_t r2 = sys_write(1, (void *)0xFFFFFFFF, 8); 
    int32_t r3 = sys_write(1, "[B] ok\n", 7);

    if (r1 < 0 && r2 < 0 && r3 == 7) 
        sys_exit(0); 
    else 
        sys_exit(1); 
    return 0; 
}
