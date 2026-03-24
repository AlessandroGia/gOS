enable_language(ASM)
find_program(CMAKE_OBJCOPY objcopy REQUIRED)

set(KERNEL_ELF_DIR ${CMAKE_BINARY_DIR}/kernel/elf)
set(KERNEL_BIN_DIR ${CMAKE_BINARY_DIR}/kernel/bin)
set(KERNEL_GC_DIR ${CMAKE_BINARY_DIR}/kernel/gc)

add_executable(kernel_elf
    ${CMAKE_CURRENT_SOURCE_DIR}/core/kmain.c

    ${CMAKE_CURRENT_SOURCE_DIR}/core/console/console.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/console/font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/memory_map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/memory/memory.c

    ${CMAKE_CURRENT_SOURCE_DIR}/core/panic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/early_allocator.c

    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/video/framebuffer.c

    ${CMAKE_CURRENT_SOURCE_DIR}/lib/memory/kmemset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/memory/kmemcpy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/memory/kmemmov.c

    ${CMAKE_CURRENT_SOURCE_DIR}/lib/string/kstrcpy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/string/kstrcmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/string/kstrlen.c

    ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/boot/entry.S
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/memory/paging.c
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
    -T${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/linker.ld
)

set_target_properties(kernel_elf PROPERTIES
    OUTPUT_NAME kernel
    PREFIX ""
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY ${KERNEL_ELF_DIR}
)

#set(KERNEL_BIN ${KERNEL_BIN_DIR}/kernel.bin)
set(KERNEL_GC ${KERNEL_GC_DIR}/kernel.GC)

#add_custom_command(
#    OUTPUT ${KERNEL_BIN}
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${KERNEL_BIN_DIR}
#    COMMAND ${CMAKE_OBJCOPY}
#        -O binary
#        $<TARGET_FILE:kernel_elf>
#        ${KERNEL_BIN}
#    DEPENDS kernel_elf mkgc
#    VERBATIM
#)

#add_custom_command(
#    OUTPUT ${KERNEL_GC}
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${KERNEL_GC_DIR}
#    COMMAND $<TARGET_FILE:mkgc>
#        ${KERNEL_BIN}
#        ${KERNEL_GC}
#    DEPENDS ${KERNEL_BIN} mkgc
#    VERBATIM
#)

add_custom_command(
    OUTPUT ${KERNEL_GC}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${KERNEL_GC_DIR}
    COMMAND $<TARGET_FILE:mkgc>
        $<TARGET_FILE:kernel_elf>
        ${KERNEL_GC}
    DEPENDS kernel_elf mkgc
    VERBATIM
)

add_custom_target(kernel ALL DEPENDS ${KERNEL_GC})
