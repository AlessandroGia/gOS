#ifndef KERNELLOADER_H
#define KERNELLOADER_H

#include <efi/efi.h>

EFI_STATUS load_kernel_file(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *st,
    VOID **kernel_buffer,
    UINTN *kernel_size);

void copy_kernel_to_address(
    VOID *kernel_buffer,
    UINTN kernel_size,
    VOID *destination);

#endif
