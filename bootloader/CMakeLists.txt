find_program(CMAKE_OBJCOPY objcopy REQUIRED)

set(GNUEFI_INC       /usr/include/efi)
set(GNUEFI_INC_ARCH  /usr/include/efi/x86_64)
set(GNUEFI_INC_PROTO /usr/include/efi/protocol)
set(GNUEFI_CRT       /usr/lib/crt0-efi-x86_64.o)
set(GNUEFI_LDS       /usr/lib/elf_x86_64_efi.lds)

set(BOOTLOADER_SO_DIR  ${CMAKE_BINARY_DIR}/bootloader/so)
set(BOOTLOADER_EFI_DIR ${CMAKE_BINARY_DIR}/bootloader/efi)

set(UEFI_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GNUEFI_INC}
    ${GNUEFI_INC_ARCH}
    ${GNUEFI_INC_PROTO}
)

set(UEFI_COMPILE_OPTIONS
    -ffreestanding
    -fno-stack-protector
    -fpic
    -fshort-wchar
    -mno-red-zone
    -Wall
    -Wextra
)

set(UEFI_LINK_OPTIONS
    -nostdlib
    -shared
    -fshort-wchar
    -Wl,-znocombreloc
    -Wl,-T,${GNUEFI_LDS}
    -Wl,-Bsymbolic
    -Wl,--defsym=EFI_SUBSYSTEM=10
)

function(add_uefi_app target_name output_name)
    add_library(${target_name}_so SHARED ${ARGN})

    target_include_directories(${target_name}_so PRIVATE ${UEFI_INCLUDE_DIRS})
    target_compile_options(${target_name}_so PRIVATE ${UEFI_COMPILE_OPTIONS})
    target_link_options(${target_name}_so PRIVATE ${UEFI_LINK_OPTIONS})

    target_link_libraries(${target_name}_so PRIVATE
        ${GNUEFI_CRT}
        efi
        gnuefi
    )

    set_target_properties(${target_name}_so PROPERTIES
        OUTPUT_NAME ${target_name}
        PREFIX ""
        SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY ${BOOTLOADER_SO_DIR}
    )

    set(efi_file ${BOOTLOADER_EFI_DIR}/${output_name})

    add_custom_command(
        OUTPUT ${efi_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BOOTLOADER_EFI_DIR}
        COMMAND ${CMAKE_OBJCOPY}
            -j .text
            -j .sdata
            -j .data
            -j .dynamic
            -j .dynsym
            -j .rel
            -j .rela
            -j .reloc
            --target=efi-app-x86-64
            $<TARGET_FILE:${target_name}_so>
            ${efi_file}
        DEPENDS ${target_name}_so
        VERBATIM
    )

    add_custom_target(${target_name} ALL DEPENDS ${efi_file})
endfunction()


add_uefi_app (
    bootx64
    BootX64.efi
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/boot/bootx64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/console/menu/menu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/memory/memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/helper/helper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/console/input.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/image/image.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/console/screen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/power/power.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/boot/actions.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/boot/menu/memory/memory.c
)

add_uefi_app (
    osloader
    OsLoader.efi
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/apps/loader/osloader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/memory/memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/common/helper/helper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/apps/loader/framebuffer/framebuffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/apps/loader/handoff/handoff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uefi/apps/loader/kernel/bin/kernel.c
)
