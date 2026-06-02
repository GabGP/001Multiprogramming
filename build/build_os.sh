#!/bin/bash

# Build Script for OS
# Exit immediately if a command exits with a non-zero status
set -e

# Run from script directory so paths work from anywhere
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
mkdir -p bin # Ensure bin directory exists for output files

TARGET="${TARGET:-versatilepb}"   # default target
echo "  Selected TARGET=${TARGET}"

# Define commands and flags
CC="arm-none-eabi-gcc"
AS="arm-none-eabi-as"
LD="arm-none-eabi-ld"
OBJCOPY="arm-none-eabi-objcopy"

case "$TARGET" in
  versatilepb)
    AFLAGS="-g"
    CFLAGS="-g -O0 -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -Wall -nostdlib -nostartfiles -ffreestanding -DPLATFORM_VERSATILEPB"
    LDFLAGS="-T ../OS/linker.ld --defsym=MEM_ADDR=0x00000000 --defsym=P1_ADDR=0x00100000 --defsym=P2_ADDR=0x00200000"
    if [ "$DEBUG" = "1" ]; then
      RUN_CMD="qemu-system-arm -M versatilepb -cpu cortex-a8 -nographic -kernel bin/os.elf -S -gdb tcp::3333"
    else
      RUN_CMD="qemu-system-arm -M versatilepb -cpu cortex-a8 -nographic -kernel bin/os.elf"
    fi
    ;;
  beaglebone)
    AFLAGS=""
    CFLAGS="-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -DPLATFORM_BEAGLEBONE"
    LDFLAGS="-T ../OS/linker.ld --defsym=MEM_ADDR=0x82000000 --defsym=P1_ADDR=0x82100000 --defsym=P2_ADDR=0x82200000"
    RUN_CMD=""  # none, since we will run on real hardware
    ;;
  *)
    echo "Unknown target: $TARGET"
    exit 1
    ;;
esac

# Remove previous compiled objects and binaries
echo "  Cleaning up previous build files..."
rm -f bin/*.o bin/os.elf bin/os.bin

# Compile processes first so we can embed them in the OS
TARGET=$TARGET ./build_process_1.sh
TARGET=$TARGET ./build_process_2.sh

echo ""
echo "Building OS..."

echo "  Assembling root.s..."
$AS $AFLAGS -o bin/root.o ../OS/root.s

echo "  Assembling processes.s (embedding P1 and P2 binaries)..."
$AS $AFLAGS -o bin/processes.o ../OS/processes.s

echo "  Compiling OS..."
$CC -c $CFLAGS -o bin/kernel.o ../OS/kernel.c
$CC -c $CFLAGS -o bin/dispatcher.o ../OS/dispatcher.c
$CC -c $CFLAGS -o bin/scheduler.o ../OS/scheduler/scheduler.c
$CC -c $CFLAGS -o bin/queue.o ../OS/scheduler/queue.c

echo "  Compiling drivers..."
$CC -c $CFLAGS -o bin/intc.o ../drivers/intc.c
$CC -c $CFLAGS -o bin/timer.o ../drivers/timer.c
$CC -c $CFLAGS -o bin/uart.o ../drivers/uart.c

echo "  Compiling libraries..."
$CC -c $CFLAGS -o bin/stdio.o ../lib/stdio.c
$CC -c $CFLAGS -o bin/stdlib.o ../lib/stdlib.c

echo "  Compiling debugger..."
$CC -c $CFLAGS -o bin/logs.o ../debugger/logs.c

echo "  Linking object files..."
$LD $LDFLAGS -o bin/os.elf \
    bin/root.o \
    bin/processes.o \
    bin/kernel.o \
    bin/dispatcher.o \
    bin/scheduler.o \
    bin/queue.o \
    bin/intc.o \
    bin/timer.o \
    bin/uart.o \
    bin/stdio.o \
    bin/stdlib.o \
    bin/logs.o

echo "  Converting ELF to binary..."
$OBJCOPY -O binary bin/os.elf bin/os.bin

echo "  Disassembling for debugging..."
arm-none-eabi-objdump -d bin/os.elf > bin/disasm.txt

echo "  Cleaning up compilation files..."
rm -f bin/*.o

if [ "$TARGET" = "versatilepb" ]; then
  echo "  Build complete for VerstatilePB."
  $RUN_CMD
else
  echo "  Build complete for BeagleBone."
fi
