# ARM-Based Multiprogramming OS

#### By @23004004 - Carlos Alvarez and @GabGP - Gabriel Garcia

This project implements a basic multiprogramming operating system capable of performing context switches between multiple processes using a Round-Robin scheduler. The system is designed to run on ARM-based architectures, specifically supporting **VersatilePB** - ARM926EJ-S and **BeagleBone Black** - ARM Cortex-A8 (AM335x).

## Features

- **Round-Robin Scheduling**: Implements a preemptive scheduling algorithm that assigns a fixed time unit (quantum) to each process in the ready queue, ensuring fair CPU distribution and preventing process starvation.
- **PCB Context Management**: Utilizes a structured Process Control Block to save and restore the full CPU state (R0–R12, SP, LR, PC, and SPSR) during context switches, allowing seamless multitasking between independent tasks.
- **Banked Register Handling**: Leverages ARM's banked register architecture to safely transition between exception modes (IRQ/SVC) and System/User mode, maintaining stack pointer integrity and strict process isolation.
- **Millisecond Timer Support**: Integrated hardware timer drivers for VersatilePB and BeagleBone Black that generate periodic interrupts, serving as the fundamental timing mechanism for the scheduler's preemption.

## How to Run?

### To build

- `make bbb` BeagleBone Black
- `make qemu` VersatilePB - Qemu
- `make qemu-debug` VersatilePB - Qemu with GDB

Note: This program is meant to be ran on Linux and requires gcc-arm-none-aebi , gdb-multiarch and optionally qemu.

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

## Process States

The kernel manages the lifecycle of each task through a set of defined states stored in the PCB.

| State | Value | Description |
| :--- | :--- | :--- |
| `PROCESS_NEW` | 0 | The process is being created and its PCB is being initialized. |
| `PROCESS_READY` | 1 | The process is waiting in the `ready_queue` to be assigned to the CPU. |
| `PROCESS_RUNNING` | 2 | The process instructions are currently being executed by the processor. |
| `PROCESS_WAITING` | 3 | The process is waiting for an event to occur (e.g., yielding or syscall handling). |
| `PROCESS_SUSPENDED` | 4 | The process execution is paused but the context is preserved in memory. |
| `PROCESS_TERMINATED` | 5 | The process has completed execution or has been aborted due to a fault. |

## Syscall ABI

The system uses registers **R0-R3** for system call interfacing. **R0** is used to pass the Syscall ID and also stores the return code upon completion.

| Syscall Name | ID (R0) | Arg 1 (R1) | Arg 2 (R2) | Arg 3 (R3) | Return Code (R0) | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `SYS_YIELD` | 1 | - | - | - | `SUCCESS_RC` | Voluntarily yield CPU to the next process. |
| `SYS_EXIT` | 2 | Exit Code | - | - | Exit Code | Terminate the current process with an exit code. |
| `SYS_WRITE` | 3 | File Descriptor | Buffer | Length | Bytes / Error | Write string to UART (FD 1 supported). |

### Return and Error Codes

| Return Code | Value | Description |
| :--- | :--- | :--- |
| `SUCCESS_RC` | 0 | Operation completed successfully. |
| `INVALID_SYSCALL_RC` | -1 | The requested Syscall ID is not recognized. |
| `INVALID_ARGUMENT_RC` | -2 | Provided arguments are invalid. (FD != 1or 0 > Length > 256) |
| `INVALID_USR_PTR_RC` | -3 | Memory access violation (Buffer outside process boundary). |


**Unknown Syscall Behavior**  
If a process requests an unknown Syscall ID, the kernel will immediately transition the process to the `PROCESS_TERMINATED` state, set the return register to `INVALID_SYSCALL_RC`, and trigger the scheduler to pick the next available task.

## Fault Handling Policy

When a hardware exception or processor fault occurs, the `fault_dispatcher` categorizes the error and enforces a system safety policy.

| Fault Classification / Type | Value | Outcome |
| :--- | :--- | :--- |
| `NO_FAULT` | 0 | **-** |
| `INVALID_MAPPING_FAULT` (Page Fault) | 1 | **Terminate** |
| `PRIVILEGE_VIOLATION_FAULT` | 2 | **Terminate** |
| `ALIGNMENT_ERROR_FAULT` | 3 | **Terminate** |
| `PERMISSION_FAULT` | 4 | **Terminate** |
| `UNKNOWN_FAULT` | 5 | **Terminate** |

**Recovery Strategy**  
The current kernel policy is **Fail-Stop** for individual processes. If any fault is detected:

1. The specific PID is marked as `PROCESS_TERMINATED`.
2. The `fault_type` and `termination_reason` are recorded in the process's PCB.
3. The scheduler performs swaps out the current process and selects the next `PROCESS_READY` process.
4. The system continues running other processes.

## Termination Reason

| Termination Reason | Value | Description |
| :--- | :--- | :--- |
| `NORMAL_EXIT` | 0 | Process terminated normally. |
| `SYSCALL_EXIT` | 1 | Process terminated after a syscall (successful or failed). |
| `FAULT_EXIT` | 2 | Process terminated after a fault. |
