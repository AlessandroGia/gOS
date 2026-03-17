#!/usr/bin/env bash
set -e

mkdir -p testfs/EFI/BOOT
cp build/bootloader/efi/BootX64.efi testfs/EFI/BOOT/BootX64.efi
cp build/bootloader/efi/OsLoader.efi testfs/EFI/BOOT/OsLoader.efi

cp build/kernel/bin/kernel.bin testfs/kernel.bin

qemu-system-x86_64 \
  -machine q35 \
  -m 256M \
  -drive if=pflash,format=raw,readonly=on,file=assets/OVMF_CODE.fd \
  -drive format=raw,file=fat:rw:testfs
