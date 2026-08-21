#!/bin/bash

# Build Script for Process 2
# Exit immediately if a command exits with a non-zero status
set -e

# Run from script directory so paths work from anywhere
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
mkdir -p bin obj/p2 # Ensure output and object directories exist

echo
echo "Building Process 2..."

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
    LDFLAGS="-T ../src/user/p2/linker.ld --defsym=P2_ADDR=0x00300000"
    ;;
  beaglebone)
    CFLAGS="-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -Wall -nostdlib -nostartfiles -ffreestanding -DPLATFORM_BEAGLEBONE -I../src/include"
    LDFLAGS="-T ../src/user/p2/linker.ld --defsym=P2_ADDR=0x82300000"
    ;;
  *)
    echo "Unknown target: $TARGET"
    exit 1
    ;;
esac

# Remove previous compiled objects and binaries
echo "  Cleaning up previous build files..."
rm -f obj/p2/*.o bin/process_2.elf bin/process_2.bin

echo "  Assembling root.s..."
$AS -o obj/p2/root.o ../src/user/p2/root.s

echo "  Compiling process_2.c..."
$CC -c $CFLAGS -o obj/p2/process_2.o ../src/user/p2/process_2.c

echo "  Linking object files..."
$LD $LDFLAGS -o bin/process_2.elf obj/p2/root.o obj/p2/process_2.o

echo "  Converting ELF to binary..."
$OBJCOPY -O binary bin/process_2.elf bin/process_2.bin

if [ "$TARGET" = "versatilepb" ]; then
  echo "  Build complete for VerstatilePB (QEMU)."
else
  echo "  Build complete for BeagleBone."
fi
