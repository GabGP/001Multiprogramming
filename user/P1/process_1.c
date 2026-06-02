#include "../../lib/stdio.h"
#include "../../lib/syscalls.h"

#define EXPECTED_ERR_ARG  -2
#define EXPECTED_ERR_PTR  -3

int process_1(void) 
{
    int r1 = sys_write(9, "X\n", 2); 
    int r2 = sys_write(1, (void *)0xFFFFFFFF, 8); 
    int r3 = sys_write(1, "[A] Protections Verified!\n", 26);

    if (r1 != EXPECTED_ERR_ARG || r2 != EXPECTED_ERR_PTR || r3 != 26) 
    {
        sys_exit(1); 
        return 0;
    }

    char msg[] = "[B] tick XX\n"; 

    for (int i = 0; i < 20; i++) 
    {
        int len;
        if (i < 10) 
        {
            msg[9] = '0' + i;
            msg[10] = '\n';
            len = 11;
        } 
        else 
        {
            msg[9] = '0' + (i / 10);
            msg[10] = '0' + (i % 10);
            msg[11] = '\n';
            len = 12;
        }

        int n = sys_write(1, msg, len); 
        if (n < 0) 
        {
            sys_exit(2); 
            return 0;
        }

        sys_yield(); 
    }

    sys_exit(0);
    return 0; 
}