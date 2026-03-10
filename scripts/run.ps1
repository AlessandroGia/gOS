New-Item -ItemType Directory -Force -Path "testfs\EFI\BOOT" | Out-Null
Copy-Item "build\BOOTX64.EFI" "testfs\EFI\BOOT\BOOTX64.EFI" -Force

qemu-system-x86_64 `
  -machine q35 `
  -m 256M `
  -drive if=pflash,format=raw,readonly=on,file=assets/OVMF_CODE.fd `
  -drive format=raw,file=fat:rw:testfs
