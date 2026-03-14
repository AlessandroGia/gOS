cmake_minimum_required(VERSION 3.20)
project(uefi_loader C)

set(CMAKE_C_STANDARD 11)

set(GNUEFI_INC /usr/include/efi)
set(GNUEFI_INC_ARCH /usr/include/efi/x86_64)
set(GNUEFI_INC_PROTO /usr/include/efi/protocol)


function(add_uefi_app target_name output_name)
    set(obj_files "")
    set(all_sources ${ARGN})

    foreach(source_file IN LISTS all_sources)
        get_filename_component(source_name ${source_file} NAME_WE)
        set(obj_file ${CMAKE_BINARY_DIR}/${target_name}_${source_name}.o)

        add_custom_command(
            OUTPUT ${obj_file}
            COMMAND gcc
                    -ffreestanding
                    -fno-stack-protector
                    -fpic
                    -fshort-wchar
                    -mno-red-zone
                    -Wall
                    -Wextra
                    -I${CMAKE_SOURCE_DIR}
                    -I${CMAKE_SOURCE_DIR}/src
                    -I${GNUEFI_INC}
                    -I${GNUEFI_INC_ARCH}
                    -I${GNUEFI_INC_PROTO}
                    -c ${source_file}
                    -o ${obj_file}
            DEPENDS ${source_file}
            VERBATIM
        )

        list(APPEND obj_files ${obj_file})
    endforeach()

    set(efi_file ${CMAKE_BINARY_DIR}/${output_name})
    set(so_file  ${CMAKE_BINARY_DIR}/${target_name}.so)

    add_custom_command(
        OUTPUT ${so_file}
        COMMAND gcc
                -nostdlib
                -Wl,-znocombreloc
                -Wl,-T,/usr/lib/elf_x86_64_efi.lds
                -shared
                -fshort-wchar
                -Wl,-Bsymbolic
                -Wl,--defsym=EFI_SUBSYSTEM=10
                /usr/lib/crt0-efi-x86_64.o
                ${obj_files}
                -o ${so_file}
                -L/usr/lib
                -lefi
                -lgnuefi
        DEPENDS ${obj_files}
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${efi_file}
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
            ${so_file}
            ${efi_file}
        DEPENDS ${so_file}
        VERBATIM
    )

    add_custom_target(${target_name} ALL DEPENDS ${efi_file})
endfunction()

add_uefi_app (
    bootx64
    BootX64.efi
    ${CMAKE_SOURCE_DIR}/src/uefi/boot/bootx64.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/memory/memory.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/input/input.c
)

add_uefi_app (
    osloader
    OsLoader.efi
    ${CMAKE_SOURCE_DIR}/src/uefi/loader/osloader.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/memory/memory.c
)

set(CFLAGS_KERNEL
    -ffreestanding
    -fno-stack-protector
    -mno-red-zone
    -Wall
    -Wextra
)

set(SRC_KERNEL ${CMAKE_SOURCE_DIR}/kernel/kernel.c)
set(OBJ_KERNEL ${CMAKE_BINARY_DIR}/kernel.o)
set(ELF_KERNEL ${CMAKE_BINARY_DIR}/kernel.elf)
set(BIN_KERNEL ${CMAKE_BINARY_DIR}/kernel.bin)

add_custom_command(
    OUTPUT ${OBJ_KERNEL}
    COMMAND gcc ${CFLAGS_KERNEL}
            -I${CMAKE_SOURCE_DIR}
            -c ${SRC_KERNEL}
            -o ${OBJ_KERNEL}
    DEPENDS ${SRC_KERNEL}
    VERBATIM
)

add_custom_command(
    OUTPUT ${ELF_KERNEL}
    COMMAND ld
            -nostdlib
            -T ${CMAKE_SOURCE_DIR}/linker/kernel.ld
            ${OBJ_KERNEL}
            -o ${ELF_KERNEL}
    DEPENDS ${OBJ_KERNEL} ${CMAKE_SOURCE_DIR}/linker/kernel.ld
    VERBATIM
)

add_custom_command(
    OUTPUT ${BIN_KERNEL}
    COMMAND objcopy
            -O binary
            ${ELF_KERNEL}
            ${BIN_KERNEL}
    DEPENDS ${ELF_KERNEL}
    VERBATIM
)

add_custom_target(kernel ALL DEPENDS ${ELF_KERNEL} ${BIN_KERNEL})
