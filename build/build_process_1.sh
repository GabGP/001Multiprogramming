#!/bin/bash

# Build Script for Process 1
# Exit immediately if a command exits with a non-zero status
set -e

# Run from script directory so paths work from anywhere
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
mkdir -p bin obj/p1 # Ensure output and object directories exist

echo
echo "Building Process 1..."

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
    LDFLAGS="-T ../src/user/p1/linker.ld --defsym=P1_ADDR=0x00200000"
    ;;
  beaglebone)
    CFLAGS="-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -Wall -nostdlib -nostartfiles -ffreestanding -DPLATFORM_BEAGLEBONE -I../src/include"
    LDFLAGS="-T ../src/user/p1/linker.ld --defsym=P1_ADDR=0x82200000"
    ;;
  *)
    echo "Unknown target: $TARGET"
    exit 1
    ;;
esac

# Remove previous compiled objects and binaries
echo "  Cleaning up previous build files..."
rm -f obj/p1/*.o bin/process_1.elf bin/process_1.bin

echo "  Assembling root.s..."
$AS -o obj/p1/root.o ../src/user/p1/root.s

echo "  Compiling process_1.c..."
$CC -c $CFLAGS -o obj/p1/process_1.o ../src/user/p1/process_1.c

echo "  Linking object files..."
$LD $LDFLAGS -o bin/process_1.elf obj/p1/root.o obj/p1/process_1.o

echo "  Converting ELF to binary..."
$OBJCOPY -O binary bin/process_1.elf bin/process_1.bin

if [ "$TARGET" = "versatilepb" ]; then
  echo "  Build complete for VerstatilePB (QEMU)."
else
  echo "  Build complete for BeagleBone."
fi
