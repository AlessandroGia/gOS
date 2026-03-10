cmake_minimum_required(VERSION 3.20)
project(uefi_loader C)

set(CMAKE_C_STANDARD 11)

set(GNUEFI_INC /usr/include/efi)
set(GNUEFI_INC_ARCH /usr/include/efi/x86_64)
set(GNUEFI_INC_PROTO /usr/include/efi/protocol)

set(CFLAGS
    -ffreestanding
    -fno-stack-protector
    -fpic
    -fshort-wchar
    -mno-red-zone
    -Wall
    -Wextra
)

set(LDFLAGS
    -nostdlib
    -znocombreloc
    -T /usr/lib/elf_x86_64_efi.lds
    -shared
    -Bsymbolic
    /usr/lib/crt0-efi-x86_64.o
)

set(SRC_BOOT ${CMAKE_SOURCE_DIR}/src/uefi/boot/bootx64.c)
set(OBJ_BOOT ${CMAKE_BINARY_DIR}/build/bootx64.o)
set(SO_BOOT  ${CMAKE_BINARY_DIR}/build/bootx64.so)
set(EFI_BOOT ${CMAKE_BINARY_DIR}/build/BOOTX64.EFI)

add_custom_command(
    OUTPUT ${OBJ_BOOT}
    COMMAND gcc ${CFLAGS}
            -I${GNUEFI_INC}
            -I${GNUEFI_INC_ARCH}
            -I${GNUEFI_INC_PROTO}
            -c ${SRC_BOOT}
            -o ${OBJ_BOOT}
    DEPENDS ${SRC_BOOT}
    VERBATIM
)

add_custom_command(
    OUTPUT ${SO_BOOT}
    COMMAND ld ${LDFLAGS}
            ${OBJ_BOOT}
            -o ${SO_BOOT}
            -lefi
            -lgnuefi
    DEPENDS ${OBJ_BOOT}
    VERBATIM
)

add_custom_command(
    OUTPUT ${EFI_BOOT}
    COMMAND objcopy
        -j .text
        -j .sdata
        -j .data
        -j .dynamic
        -j .dynsym
        -j .rel
        -j .rela
        -j .reloc
        --target=efi-app-x86-64
        ${SO_BOOT}
        ${EFI_BOOT}
    DEPENDS ${SO_BOOT}
    VERBATIM
)

add_custom_target(bootloader ALL DEPENDS ${EFI_BOOT})

#set(SRC_KERNEL ${CMAKE_SOURCE_DIR}/src/kernel/kernel.c)
#set(OBJ_KERNEL ${CMAKE_BINARY_DIR}/build/kernel.o)
#set(SO_KERNEL  ${CMAKE_BINARY_DIR}/build/kernel.so)
#set(EFI_KERNEL ${CMAKE_BINARY_DIR}/build/KERNEL.EFI)

#add_custom_command(
#    OUTPUT ${OBJ_KERNEL}
#    COMMAND gcc ${CFLAGS}
#            -I${GNUEFI_INC}
#            -I${GNUEFI_INC_ARCH}
#            -I${GNUEFI_INC_PROTO}
#            -c ${SRC_KERNEL}
#            -o ${OBJ_KERNEL}
#    DEPENDS ${SRC_KERNEL}
#    VERBATIM
#)

#add_custom_command(
#    OUTPUT ${SO_KERNEL}
#    COMMAND ld ${LDFLAGS}
#            ${OBJ_KERNEL}
#            -o ${SO_KERNEL}
#            -lefi
#            -lgnuefi
#    DEPENDS ${OBJ_KERNEL}
#    VERBATIM
#)

#add_custom_command(
#    OUTPUT ${EFI_KERNEL}
#    COMMAND objcopy
#        -j .text
#        -j .sdata
#        -j .data
#        -j .dynamic
#        -j .dynsym
#        -j .rel
#        -j .rela
#        -j .reloc
#        --target=efi-app-x86-64
#        ${SO_KERNEL}
#        ${EFI_KERNEL}
#    DEPENDS ${SO_KERNEL}
#    VERBATIM
#)

#add_custom_target(kernel ALL DEPENDS ${EFI_KERNEL})
