#include <efi/efi.h>
#include <efi/efilib.h>

#include "memory.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *SystemTable, PMEMORY_MAP *mem_map)
{

    EFI_STATUS Status;
    BOOLEAN allocated_struct = FALSE;

    if (mem_map == NULL)
        return EFI_INVALID_PARAMETER;

    if (*mem_map == NULL)
    {
        Status = uefi_call_wrapper(
            SystemTable->BootServices->AllocatePool, 3,
            EfiLoaderData,
            sizeof(MEMORY_MAP),
            (VOID **)mem_map);
        if (EFI_ERROR(Status))
        {
            LOG_ERROR("AllocatePool for MEMORY_MAP failed: %r", Status);
            goto cleanup;
        }
        allocated_struct = TRUE;

        (*mem_map)->MemoryMap = NULL;
        (*mem_map)->MemoryMapSize = 0;
        (*mem_map)->MapKey = 0;
        (*mem_map)->DescriptorSize = 0;
        (*mem_map)->DescriptorVersion = 0;
    }

    if ((*mem_map)->MemoryMap != NULL)
    {
        free_pool(SystemTable, (*mem_map)->MemoryMap);
        (*mem_map)->MemoryMap = NULL;
    }

    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap, 5,
        &(*mem_map)->MemoryMapSize,
        (*mem_map)->MemoryMap,
        &(*mem_map)->MapKey,
        &(*mem_map)->DescriptorSize,
        &(*mem_map)->DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL)
    {
        LOG_ERROR("Initial GetMemoryMap failed: %r", Status);
        goto cleanup;
    }

    (*mem_map)->MemoryMapSize += (*mem_map)->DescriptorSize * 2;

    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePool, 3,
        EfiLoaderData,
        (*mem_map)->MemoryMapSize,
        (VOID **)&(*mem_map)->MemoryMap);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("AllocatePool for memory map buffer failed: %r", Status);
        goto cleanup;
    }

    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap, 5,
        &(*mem_map)->MemoryMapSize,
        (*mem_map)->MemoryMap,
        &(*mem_map)->MapKey,
        &(*mem_map)->DescriptorSize,
        &(*mem_map)->DescriptorVersion);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Second GetMemoryMap failed: %r", Status);
        goto cleanup;
    }

    return EFI_SUCCESS;

cleanup:
    if (*mem_map != NULL)
    {
        if ((*mem_map)->MemoryMap != NULL)
        {
            free_pool(SystemTable, (*mem_map)->MemoryMap);
            (*mem_map)->MemoryMap = NULL;
        }

        if (allocated_struct)
        {
            free_pool(SystemTable, *mem_map);
            *mem_map = NULL;
        }
    }

    return Status;
}
