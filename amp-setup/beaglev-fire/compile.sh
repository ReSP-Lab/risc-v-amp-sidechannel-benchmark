#!/bin/bash
set -eu

CC=riscv64-unknown-linux-gnu-
GCC="$CC"gcc
OBJCOPY="$CC"objcopy
OBJDUMP="$CC"objdump
CFLAGS="-march=rv64gc_zifencei -mcmodel=medany -nostdlib -ffreestanding -fno-pie -no-pie"

$GCC $CFLAGS -T linker_loader.ld loader.c -o amp_loader.elf
$OBJCOPY -O binary amp_loader.elf amp_loader.bin
#$OBJDUMP -S amp_loader.elf > bv-fire_amp-payload.s

set -x
$HSSGEN -c hss-amp.yaml -v -v amp-payload.bin
$HSSGEN -d amp-payload.bin
