#include "mpu.h"

// The first-level TTB must be strictly aligned to 16KB boundaries
unsigned int page_table[TTB_ENTRIES] __attribute__((aligned(16384)));

// Function to initialize the MPU with the defined memory regions and permissions
void mpu_init(void)
{
    for (unsigned int i = 0; i < TTB_ENTRIES; i++)
    {
        unsigned int physical_address = i << 20; // Shift to establish 1MB blocks (Sections)

        // Mark the entry as a Section (bits [1:0] = 10)
        unsigned int entry = physical_address | 0x02;

        if (physical_address >= KERNEL_START && physical_address <= KERNEL_END)
        {
            entry |= (1 << 10); // AP[2:0] = 001, Kernel: Read/Write, User: No Access
        }
        else if (physical_address >= USER_START && physical_address <= USER_END)
        {
            entry |= (3 << 10); // AP[2:0] = 011, Kernel: Read/Write, User: Read/Write
        }
        else
        {
            // Peripheral/IO space or Dead Zones
            entry |= (1 << 10); // AP[2:0] = 001, Kernel: Read/Write, User: No Access
        }

        page_table[i] = entry;
    }

    enable_mmu();
}
