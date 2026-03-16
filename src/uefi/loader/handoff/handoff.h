#ifndef HANDOFF_H
#define HANDOFF_H

#include <efi/efi.h>

#include "shared/bootinfo.h"

#include "uefi/common/memory/memory.h"

EFI_STATUS exit_boot_services_with_retry(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    BootInfo *boot_info);

void jump_to_kernel(
    EFI_PHYSICAL_ADDRESS kernel_entry_point,
    BootInfo *boot_info);

#endif
