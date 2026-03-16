#include <efi/efi.h>

#include "handoff.h"

#include "shared/bootinfo.h"

#include "uefi/common/memory/memory.h"
#include "uefi/common/log/log.h"

EFI_STATUS exit_boot_services_with_retry(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    BootInfo *boot_info)
{
    EFI_STATUS Status;
    PMEMORY_MAP mem_map = NULL;

    if (boot_info == NULL)
        return EFI_INVALID_PARAMETER;

    if (EFI_ERROR(get_memory_map(SystemTable, &mem_map)))
    {
        LOG_ERROR(L"Memory map retrieval failed during retry.");
        return EFI_LOAD_ERROR;
    }
    boot_info->memory_map = mem_map->MemoryMap;
    boot_info->memory_map_size = mem_map->MemoryMapSize;
    boot_info->memory_map_descriptor_size = mem_map->DescriptorSize;

    Status = uefi_call_wrapper(
        SystemTable->BootServices->ExitBootServices,
        2,
        ImageHandle,
        mem_map->MapKey);

    if (Status == EFI_INVALID_PARAMETER)
    {
        LOG_WARN(L"ExitBootServices failed with stale MapKey, retrying.");

        if (EFI_ERROR(get_memory_map(SystemTable, &mem_map)))
        {
            LOG_ERROR(L"Memory map retrieval failed during retry.");
            return EFI_LOAD_ERROR;
        }

        boot_info->memory_map = mem_map->MemoryMap;
        boot_info->memory_map_size = mem_map->MemoryMapSize;
        boot_info->memory_map_descriptor_size = mem_map->DescriptorSize;

        Status = uefi_call_wrapper(
            SystemTable->BootServices->ExitBootServices,
            2,
            ImageHandle,
            mem_map->MapKey);
    }
    return Status;
}

void jump_to_kernel(
    EFI_PHYSICAL_ADDRESS kernel_entry_point,
    BootInfo *boot_info)
{
    typedef void (*KernelEntryPoint)(BootInfo *);
    KernelEntryPoint entry = (KernelEntryPoint)(UINTN)kernel_entry_point;
    entry(boot_info);
}
