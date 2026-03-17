enable_language(ASM)
find_program(CMAKE_OBJCOPY objcopy REQUIRED)

set(KERNEL_ELF_DIR ${CMAKE_BINARY_DIR}/kernel/elf)
set(KERNEL_BIN_DIR ${CMAKE_BINARY_DIR}/kernel/bin)

add_executable(kernel_elf
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/entry.S
)

target_include_directories(kernel_elf PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(kernel_elf PRIVATE
    -ffreestanding
    -fno-stack-protector
    -mno-red-zone
    -Wall
    -Wextra
)

target_link_options(kernel_elf PRIVATE
    -nostdlib
    -T${CMAKE_CURRENT_SOURCE_DIR}/src/linker/kernel.ld
)

set_target_properties(kernel_elf PROPERTIES
    OUTPUT_NAME kernel
    PREFIX ""
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY ${KERNEL_ELF_DIR}
)

set(KERNEL_BIN ${KERNEL_BIN_DIR}/kernel.bin)

add_custom_command(
    OUTPUT ${KERNEL_BIN}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${KERNEL_BIN_DIR}
    COMMAND ${CMAKE_OBJCOPY}
        -O binary
        $<TARGET_FILE:kernel_elf>
        ${KERNEL_BIN}
    DEPENDS kernel_elf
    VERBATIM
)

add_custom_target(kernel ALL DEPENDS ${KERNEL_BIN})
