#ifndef MEMORY_MENU_H
#define MEMORY_MENU_H

#include <efi/efi.h>

#include "uefi/common/memory/memory.h"

void show_memory_menu(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, MemoryLeakContext *mem_leak_ctx);

EFI_STATUS action_show_memory_map(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, void *context);
EFI_STATUS action_check_memory_leak(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, void *context);

#endif
