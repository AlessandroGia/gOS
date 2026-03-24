#ifndef KERNELLOADER_H
#define KERNELLOADER_H

#include <efi/efi.h>

EFI_STATUS load_kernel_file(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *st,
    VOID **kernel_buffer,
    UINTN *kernel_size);

EFI_STATUS load_kernel_to_address(
    EFI_SYSTEM_TABLE *SystemTable,
    EFI_PHYSICAL_ADDRESS *destination,
    UINTN kernel_size,
    UINTN kernel_memory_size,
    VOID *kernel_buffer);

EFI_STATUS validate_kernel_header(
    const HeaderGC *hdr,
    UINTN file_size);

#endif
