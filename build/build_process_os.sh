#!/bin/bash

# Build Script for OS Process
# Exit immediately if a command exits with a non-zero status
set -e

# Run from script directory so paths work from anywhere
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
mkdir -p bin obj/os_proc # Ensure output and object directories exist

echo
echo "Building Process OS..."

TARGET="${TARGET:-versatilepb}"   # default target
echo "  Selected TARGET=${TARGET}"

# Define commands and flags
CC="arm-none-eabi-gcc"
AS="arm-none-eabi-as"
LD="arm-none-eabi-ld"
OBJCOPY="arm-none-eabi-objcopy"

case "$TARGET" in
  versatilepb)
    CFLAGS="-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -Wall -nostdlib -nostartfiles -ffreestanding -DPLATFORM_VERSATILEPB -I../src/include"
    LDFLAGS="-T ../src/user/os/linker.ld --defsym=OS_ADDR=0x00100000"
    ;;
  beaglebone)
    CFLAGS="-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -Wall -nostdlib -nostartfiles -ffreestanding -DPLATFORM_BEAGLEBONE -I../src/include"
    LDFLAGS="-T ../src/user/os/linker.ld --defsym=OS_ADDR=0x82100000"
    ;;
  *)
    echo "Unknown target: $TARGET"
    exit 1
    ;;
esac

# Remove previous compiled objects and binaries
echo "  Cleaning up previous build files..."
rm -f obj/os_proc/*.o bin/process_os.elf bin/process_os.bin

echo "  Assembling root.s..."
$AS -o obj/os_proc/root.o ../src/user/os/root.s

echo "  Compiling process_os.c..."
$CC -c $CFLAGS -o obj/os_proc/process_os.o ../src/user/os/process_os.c

echo "  Linking object files..."
$LD $LDFLAGS -o bin/process_os.elf obj/os_proc/root.o obj/os_proc/process_os.o

echo "  Converting ELF to binary..."
$OBJCOPY -O binary bin/process_os.elf bin/process_os.bin

if [ "$TARGET" = "versatilepb" ]; then
  echo "  Build complete for VerstatilePB (QEMU)."
else
  echo "  Build complete for BeagleBone."
fi
