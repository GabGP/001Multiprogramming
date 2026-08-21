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

// The first-level TTB is defined in mpu.c and strictly aligned to 16KB boundaries
extern unsigned int page_table[TTB_ENTRIES];

extern void enable_mmu(void);

// Function to initialize the MPU with the defined memory regions and permissions
void mpu_init(void);

#endif // MPU_H
