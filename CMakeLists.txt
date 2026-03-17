cmake_minimum_required(VERSION 3.20)
project(uefi_loader C)

set(CMAKE_C_STANDARD 11)

set(GNUEFI_INC /usr/include/efi)
set(GNUEFI_INC_ARCH /usr/include/efi/x86_64)
set(GNUEFI_INC_PROTO /usr/include/efi/protocol)


function(add_uefi_app target_name output_name)
    set(obj_files "")
    set(all_sources ${ARGN})

    set(obj_dir "${CMAKE_BINARY_DIR}/obj/${target_name}")
    file(MAKE_DIRECTORY "${obj_dir}")

    foreach(source_file IN LISTS all_sources)
        file(RELATIVE_PATH rel_source "${CMAKE_SOURCE_DIR}" "${source_file}")
        get_filename_component(source_ext "${source_file}" LAST_EXT)

        set(obj_stem "${rel_source}")
        string(REPLACE "/" "_" obj_stem "${obj_stem}")
        string(REPLACE "\\" "_" obj_stem "${obj_stem}")
        string(REPLACE "${source_ext}" "" obj_stem "${obj_stem}")

        set(obj_file "${obj_dir}/${obj_stem}.o")

        add_custom_command(
            OUTPUT ${obj_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${obj_dir}"
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

    set(so_dir "${CMAKE_BINARY_DIR}/so")
    set(efi_dir "${CMAKE_BINARY_DIR}/efi")
    file(MAKE_DIRECTORY "${so_dir}")
    file(MAKE_DIRECTORY "${efi_dir}")

    set(so_file "${so_dir}/${target_name}.so")
    set(efi_file "${efi_dir}/${output_name}")

    add_custom_command(
        OUTPUT ${so_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${so_dir}"
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
        COMMAND ${CMAKE_COMMAND} -E make_directory "${efi_dir}"
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
    ${CMAKE_SOURCE_DIR}/src/uefi/common/console/menu/menu.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/memory/memory.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/helper/helper.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/console/input.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/image/image.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/console/screen.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/power/power.c
    ${CMAKE_SOURCE_DIR}/src/uefi/boot/menu/actions.c
    ${CMAKE_SOURCE_DIR}/src/uefi/memory_menu/memory_menu.c
)

add_uefi_app (
    osloader
    OsLoader.efi
    ${CMAKE_SOURCE_DIR}/src/uefi/loader/osloader.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/memory/memory.c
    ${CMAKE_SOURCE_DIR}/src/uefi/common/helper/helper.c
    ${CMAKE_SOURCE_DIR}/src/uefi/loader/framebuffer/framebuffer.c
    ${CMAKE_SOURCE_DIR}/src/uefi/loader/handoff/handoff.c
    ${CMAKE_SOURCE_DIR}/src/uefi/loader/kernel/bin/kernel.c
)

set(CFLAGS_KERNEL
    -ffreestanding
    -fno-stack-protector
    -mno-red-zone
    -Wall
    -Wextra
)

set(SRC_KERNEL ${CMAKE_SOURCE_DIR}/kernel/kernel.c)
set(SRC_KERNEL_ASM ${CMAKE_SOURCE_DIR}/kernel/entry.S)

set(OBJ_KERNEL ${CMAKE_BINARY_DIR}/kernel.o)
set(OBJ_KERNEL_ASM ${CMAKE_BINARY_DIR}/entry.o)

set(ELF_KERNEL ${CMAKE_BINARY_DIR}/kernel.elf)
set(BIN_KERNEL ${CMAKE_BINARY_DIR}/kernel.bin)

enable_language(ASM)

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
    OUTPUT ${OBJ_KERNEL_ASM}
    COMMAND gcc
            -ffreestanding
            -mno-red-zone
            -c ${SRC_KERNEL_ASM}
            -o ${OBJ_KERNEL_ASM}
    DEPENDS ${SRC_KERNEL_ASM}
    VERBATIM
)

add_custom_command(
    OUTPUT ${ELF_KERNEL}
    COMMAND ld
            -nostdlib
            -T ${CMAKE_SOURCE_DIR}/linker/kernel.ld
            ${OBJ_KERNEL_ASM}
            ${OBJ_KERNEL}
            -o ${ELF_KERNEL}
    DEPENDS ${OBJ_KERNEL_ASM} ${OBJ_KERNEL} ${CMAKE_SOURCE_DIR}/linker/kernel.ld
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
