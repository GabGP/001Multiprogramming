#ifndef MPU_H
#define MPU_H

//============================================================================
// Memory Protection Unit (MPU)
// 
// NOTE:
// The target hardware uses a Memory Management Unit (MMU).
// This file configures the MMU using an "Identity Map".
// Because no address translation occurs, this implementation functionally 
// acts as an MPU to enforce Kernel/User memory isolation boundaries.
//============================================================================

#ifdef PLATFORM_VERSATILEPB // ARM926EJ-S

// VesatilePB base address
#define MEM_ADDR 0x00000000

#elif defined(PLATFORM_BEAGLEBONE) // ARM Cortex-A8 (AM335x)

// BeagleBone Black base address
#define MEM_ADDR 0x82000000

#endif

#define KERNEL_START    (MEM_ADDR + 0x00000000)
#define KERNEL_END      (MEM_ADDR + 0x000FFFFF)
#define USER_START      (MEM_ADDR + 0x00100000)
#define USER_END        (MEM_ADDR + 0x003FFFFF)

#define TTB_ENTRIES 4096

// The first-level TTB must be strictly aligned to 16KB boundaries
unsigned int page_table[TTB_ENTRIES] __attribute__((aligned(16384)));

extern void enable_mmu();

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

#endif // MPU_H
