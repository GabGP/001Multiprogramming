# ARM-Based Multiprogramming OS

#### By @23004004 - Carlos Alvarez and @GabGP - Gabriel Garcia

This project implements a basic multiprogramming operating system capable of performing context switches between multiple processes using a Round-Robin scheduler. The system is designed to run on ARM-based architectures, specifically supporting **VersatilePB** - ARM926EJ-S and **BeagleBone Black** - ARM Cortex-A8 (AM335x).

## Features

- **Round-Robin Scheduling**: Implements a preemptive scheduling algorithm that assigns a fixed time unit (quantum) to each process in the ready queue, ensuring fair CPU distribution and preventing process starvation.
- **PCB Context Management**: Utilizes a structured Process Control Block to save and restore the full CPU state during context switches, allowing seamless multitasking between independent tasks.
- **Millisecond Timer Support**: Integrated hardware timer drivers for VersatilePB and BeagleBone Black that generate periodic interrupts, serving as the fundamental timing mechanism for the scheduler's preemption.
- **System Call Interface (ABI)**: Implements a secure gateway for user-space processes to interact with the kernel, supporting essential operations like process yielding, termination, and UART-based console output.
- **Memory Protection Unit (MPU)**: Configures the Memory Management Unit (MMU) using an identity map to function as an MPU. It enforces hardware-level isolation between Kernel and User memory regions, preventing unauthorized access to critical system resources.
- **Hardware Fault Isolation**: Includes a fault dispatcher that detects and handles various ARM exceptions, protecting system stability by terminating erroneous processes while keeping the kernel running.

## How to Run?

### To build

- `make bbb` BeagleBone Black
- `make qemu` VersatilePB - Qemu
- `make qemu-debug` VersatilePB - Qemu with GDB

**Note:** This program is meant to be ran on Linux and requires gcc-arm-none-aebi , optionally gdb-multiarch and qemu.

### To run on BeagleBone Black

While connected to a terminal like CoolTerm via UART at 1152000 bauds, run:

- `loady 0x82000000`
- Send the file at ./build/bin/os.bin
- `go 0x82000000`

## Debugging

In another terminal:

- run `gdb-multiarch build/bin/os.elf`
- `target remote localhost:3333`

Helpful debugging options

- `layout regs`
- `break <function name or number>`
- `continue`

## Technical Reference

### Kernel Log Reference

| Output | Event |
| :--- | :--- |
| `.` | Yield Scheduling |
| `...` | Normal Scheduling |
| `MODE_SWITCH KERNEL_TO_USER pid=<first> reason=initial_launch` | Initial Boot Path (Kernel → User) |
| `MODE_SWITCH USER_TO_KERNEL pid=<n> reason=timer_irq` | Interrupt Path (User → Kernel) |
| `MODE_SWITCH KERNEL_TO_USER pid=<m> reason=dispatch` | Interrupt Path (Kernel → User) |
| `MODE_SWITCH USER_TO_KERNEL pid=<n> reason=syscall id=<id>` | Syscall Path (User → Kernel) |
| `MODE_SWITCH KERNEL_TO_USER pid=<m> reason=syscall_return id=<id> rc=<rc>` | Syscall Path (Kernel → User) |
| `MODE_SWITCH USER_TO_KERNEL pid=<n> reason=fault type=<type>` | Exception Path (User → Kernel) |
| `MODE_SWITCH KERNEL_TO_USER pid=<m> reason=fault_recovery` | Exception Path (Kernel → User) |

### Scheduler Behaviour

The scheduler enforces a Round-Robin scheduling policy. It manages CPU time allocation by monitoring a `quantum` and handling process transitions based on three main triggers:

1. **Quantum Expiry**: When the `quantum` reaches zero, the current process is preempted to allow another process to run.
2. **Process Termination**: If the current process enters the `PROCESS_TERMINATED` state, a new process is immediately scheduled.
3. **Voluntary Yield**: A process can choose to give up its remaining time via the `SYS_YIELD` system call.

If any of these conditions are met, the scheduler moves to the next process in the ready queue and resets the `quantum`. When idling, the OS process will continue running.

### Process Memory Map

Each process is allocated a dedicated memory region starting at a base address determined by the platform and its PID. The entry point for each process is at the beginning of its allocated 1MB region.

| PID | Process Name   | Memory Start Address (VersatilePB) | Memory Start Address (BeagleBone Black) |
| :-- | :------------- | :--------------------------------- | :-------------------------------------- |
| 0   | Kernel         | `0x00000000`                       | `0x82000000`                            |
| 1   | OS Process     | `0x00100000`                       | `0x82100000`                            |
| 2   | User Process 1 | `0x00200000`                       | `0x82200000`                            |
| 3   | User Process 2 | `0x00300000`                       | `0x82300000`                            |
| ... | ...            | ...                                | ...                                     |

### Process Control Block (PCB)

The PCB stores the context and metadata for each process, ensuring that the system can save and restore its state during context switches.

| Variable | Description |
| :--- | :--- |
| `pid` | Unique numeric identifier for the process. |
| `state` | Current lifecycle state (e.g., READY, RUNNING, WAITING). |
| `regs[13]` | Array storing the general-purpose registers (R0 to R12). |
| `pc` | Program Counter; the address of the next instruction to be executed. |
| `sp` | Stack Pointer; points to the top of the process's private stack. |
| `lr` | Link Register; stores the return address for function calls and exceptions. |
| `spsr` | Saved Processor Status Register; used to restore the CPU mode and flags. |
| `syscall_id` | Stores the ID of the current or most recent system call. |
| `fault_type` | Records the type of hardware exception encountered by the process. |
| `termination_reason` | Indicates why a process ended (Normal exit, Syscall, or Fault). |
| `exit_code` | The status code returned to the kernel upon termination. |

#### Process States

The kernel manages the lifecycle of each task through a set of defined states stored in the PCB.

| State | Value | Description |
| :--- | :--- | :--- |
| `PROCESS_NEW` | 0 | The process is being created and its PCB is being initialized. |
| `PROCESS_READY` | 1 | The process is waiting in the `ready_queue` to be assigned to the CPU. |
| `PROCESS_RUNNING` | 2 | The process instructions are currently being executed by the processor. |
| `PROCESS_WAITING` | 3 | The process is waiting for an event to occur. |
| `PROCESS_SUSPENDED` | 4 | The process execution is paused but the context is preserved in memory. |
| `PROCESS_TERMINATED` | 5 | The process has completed execution or has been aborted due to a fault. |

### Syscall ABI

The system uses registers **R0-R3** for system call interfacing. **R0** is used to pass the Syscall ID and also stores the return code upon completion.

| Syscall Name | ID (R0) | Arg 1 (R1) | Arg 2 (R2) | Arg 3 (R3) | Return Code (R0) | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `SYS_YIELD` | 1 | - | - | - | `RC_SUCCES` | Voluntarily yield CPU to the next process. |
| `SYS_EXIT` | 2 | Exit Code | - | - | Exit Code | Terminate the current process with an exit code. |
| `SYS_WRITE` | 3 | File Descriptor | Buffer | Length | Bytes / Error | Write string to UART (FD 1 supported). |

#### Return and Error Codes

| Return Code | Value | Description |
| :--- | :--- | :--- |
| `RC_SUCCESS` | 0 | Operation completed successfully. |
| `RC_INVALID_SYSCALL` | -1 | The requested Syscall ID is not recognized. |
| `RC_INVALID_ARGUMENT` | -2 | Provided arguments are invalid. (FD != 1or 0 > Length > 256) |
| `RC_INVALID_USR_PTR` | -3 | Memory access violation (Buffer outside process boundary). |

#### Unknown Syscall Behavior

If a process requests an unknown Syscall ID, the kernel will immediately transition the process to the `PROCESS_TERMINATED` state, set the return register to `RC_INVALID_SYSCALL`, and trigger the scheduler to pick the next available task.

### Fault Handling Policy

When a hardware exception or processor fault occurs, the `fault_dispatcher` categorizes the error and enforces a system safety policy.

| Fault Classification / Type | Value | Outcome | Comment |
| :--- | :--- | :--- | :--- |
| `FAULT_NONE` | 0 | **-** | Default Value |
| `FAULT_ALIGNMENT_ERROR` | 1 | **Terminate** | MMU |
| `FAULT_ACCESS_FLAG` | 2 | **Terminate** | MMU |
| `FAULT_INVALID_MAPPING` | 3 | **Terminate** | Translation Fault / MMU |
| `FAULT_PRIV_VIOLATION` | 4 | **Terminate** | Domain Fault / MMU |
| `FAULT_PERMISSION` | 5 | **Terminate** | MMU |
| `FAULT_SYNC_EXT_ABORT` | 6 | **Terminate** | Synchronous External Abort |
| `FAULT_ASYNC_EXT_ABORT` | 7 | **Terminate** | Asynchronous External Abort |
| `FAULT_TRANS_TBL_WALK_SEA` | 8 | **Terminate** | Translation Table Walk (Sync External Abort) |
| `FAULT_TRANS_TBL_WALK_SPE` | 9 | **Terminate** | Translation Table Walk (Sync Parity Error) |
| `FAULT_MEM_ACCESS_SPE` | 10 | **Terminate** | Memory Access (Synchronous Parity Error) |
| `FAULT_MEM_ACCESS_APE` | 11 | **Terminate** | Memory Access (Asynchronous Parity Error) |
| `FAULT_DEBUG_EVENT` | 12 | **Terminate** | - |
| `FAULT_INST_CACHE_MAINT` | 13 | **Terminate** | Instruction Cache Maintenance |
| `FAULT_IMP_DEF_LD` | 14 | **Terminate** | Implementation Defined (Lockdown) |
| `FAULT_IMP_DEF_CA` | 15 | **Terminate** | Implementation Defined (Coprocessor Abort) |
| `FAULT_UNKNOWN` | -1 | **Terminate** | - |
| `FAULT_UND_INST` | -2 | **Terminate** | Undefined Instruction |

**Recovery Strategy**  

The current kernel policy is **Fail-Stop** for individual processes. If any fault is detected:

1. The current process is marked as `PROCESS_TERMINATED`.
2. The `fault_type` and `termination_reason` are recorded in the process's PCB.
3. The scheduler swaps out the current process and selects the next `PROCESS_READY` process.
4. The system continues running other processes.

**Notes:**

- Test on BeagleBone Black, otherwise the exceptions might not trigger on qemu.
- MMU faults require the MMU to be enabled and set up.

### Termination Reason

| Termination Reason | Value | Description |
| :--- | :--- | :--- |
| `EXIT_NORMAL` | 0 | Process terminated normally. Default Value. |
| `EXIT_SYSCALL` | 1 | Process terminated after a syscall (successful or failed). |
| `EXIT_FAULT` | 2 | Process terminated after a fault. |
