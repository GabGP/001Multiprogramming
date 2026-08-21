#!/bin/bash

# Build Script for OS
# Exit immediately if a command exits with a non-zero status
set -e

# Run from script directory so paths work from anywhere
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
mkdir -p bin obj/kernel # Ensure output and object directories exist

TARGET="${TARGET:-versatilepb}"   # default target
echo "  Selected TARGET=${TARGET}"

# Define commands and flags
CC="arm-none-eabi-gcc"
AS="arm-none-eabi-as"
LD="arm-none-eabi-ld"
OBJCOPY="arm-none-eabi-objcopy"

INCLUDES="-I../src/include -I../src/kernel/core -I../src/kernel/scheduler -I../src/drivers -I../src/debugger"

case "$TARGET" in
  versatilepb)
    AFLAGS="-g"
    CFLAGS="-g -O0 -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -Wall -nostdlib -nostartfiles -ffreestanding -DPLATFORM_VERSATILEPB $INCLUDES"
    LDFLAGS="-T ../src/kernel/boot/linker.ld --defsym=MEM_ADDR=0x00000000 --defsym=OS_ADDR=0x00100000 --defsym=P1_ADDR=0x00200000 --defsym=P2_ADDR=0x00300000"
    if [ "$DEBUG" = "1" ]; then
      RUN_CMD="qemu-system-arm -M versatilepb -cpu cortex-a8 -nographic -kernel bin/os.elf -S -gdb tcp::3333"
    else
      RUN_CMD="qemu-system-arm -M versatilepb -cpu cortex-a8 -nographic -kernel bin/os.elf"
    fi
    ;;
  beaglebone)
    AFLAGS=""
    CFLAGS="-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -Wall -nostdlib -nostartfiles -ffreestanding -DPLATFORM_BEAGLEBONE $INCLUDES"
    LDFLAGS="-T ../src/kernel/boot/linker.ld --defsym=MEM_ADDR=0x82000000 --defsym=OS_ADDR=0x82100000 --defsym=P1_ADDR=0x82200000 --defsym=P2_ADDR=0x82300000"
    RUN_CMD=""  # none, since we will run on real hardware
    ;;
  *)
    echo "Unknown target: $TARGET"
    exit 1
    ;;
esac

# Remove previous compiled objects and binaries
echo "  Cleaning up previous build files..."
rm -f obj/kernel/*.o bin/os.elf bin/os.bin

# Compile processes first so we can embed them in the OS
TARGET=$TARGET ./build_process_1.sh
TARGET=$TARGET ./build_process_2.sh
TARGET=$TARGET ./build_process_os.sh

echo ""
echo "Building OS..."

echo "  Assembling root.s..."
$AS $AFLAGS -o obj/kernel/root.o ../src/kernel/boot/root.s

echo "  Assembling processes.s (embedding process binaries)..."
$AS $AFLAGS -o obj/kernel/processes.o ../src/kernel/boot/processes.s

echo "  Compiling kernel..."
$CC -c $CFLAGS -o obj/kernel/kernel.o ../src/kernel/core/kernel.c
$CC -c $CFLAGS -o obj/kernel/process.o ../src/kernel/core/process.c
$CC -c $CFLAGS -o obj/kernel/dispatcher.o ../src/kernel/core/dispatcher.c
$CC -c $CFLAGS -o obj/kernel/mpu.o ../src/kernel/core/mpu.c
$CC -c $CFLAGS -o obj/kernel/scheduler.o ../src/kernel/scheduler/scheduler.c
$CC -c $CFLAGS -o obj/kernel/queue.o ../src/kernel/scheduler/queue.c

echo "  Compiling drivers..."
$CC -c $CFLAGS -o obj/kernel/intc.o ../src/drivers/intc.c
$CC -c $CFLAGS -o obj/kernel/timer.o ../src/drivers/timer.c
$CC -c $CFLAGS -o obj/kernel/uart.o ../src/drivers/uart.c

echo "  Compiling libraries..."
$CC -c $CFLAGS -o obj/kernel/stdio.o ../src/lib/stdio.c
$CC -c $CFLAGS -o obj/kernel/stdlib.o ../src/lib/stdlib.c

echo "  Compiling debugger..."
$CC -c $CFLAGS -o obj/kernel/logs.o ../src/debugger/logs.c

echo "  Linking object files..."
$LD $LDFLAGS -o bin/os.elf \
    obj/kernel/root.o \
    obj/kernel/processes.o \
    obj/kernel/kernel.o \
    obj/kernel/process.o \
    obj/kernel/dispatcher.o \
    obj/kernel/mpu.o \
    obj/kernel/scheduler.o \
    obj/kernel/queue.o \
    obj/kernel/intc.o \
    obj/kernel/timer.o \
    obj/kernel/uart.o \
    obj/kernel/stdio.o \
    obj/kernel/stdlib.o \
    obj/kernel/logs.o

echo "  Converting ELF to binary..."
$OBJCOPY -O binary bin/os.elf bin/os.bin

echo "  Disassembling for debugging..."
arm-none-eabi-objdump -d bin/os.elf > bin/disasm.txt

if [ "$TARGET" = "versatilepb" ]; then
  echo "  Build complete for VerstatilePB (QEMU)."
  $RUN_CMD
else
  echo "  Build complete for BeagleBone."
fi
