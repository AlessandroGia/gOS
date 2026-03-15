#include <efi/efi.h>
#include <efi/efilib.h>

#include "memory.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

static CHAR16 *convert(UINTN byte)
{
    if (byte > 1024 * 1024 * 1024)
    {
        UINTN gb = byte / (1024 * 1024 * 1024);
        UINTN remainder = byte % (1024 * 1024 * 1024);
        UINTN mb = remainder / (1024 * 1024);
        return PoolPrint(L"%u GB %u MB", gb, mb);
    }
    else if (byte > 1024 * 1024)
    {
        UINTN mb = byte / (1024 * 1024);
        UINTN remainder = byte % (1024 * 1024);
        UINTN kb = remainder / 1024;
        return PoolPrint(L"%u MB %u KB", mb, kb);
    }
    else if (byte > 1024)
    {
        UINTN kb = byte / 1024;
        UINTN remainder = byte % 1024;
        return PoolPrint(L"%u KB %u B", kb, remainder);
    }
    else
    {
        return PoolPrint(L"%u B", byte);
    }
}

void prov(EFI_SYSTEM_TABLE *SystemTable)
{

    /*
    PMEMORY_MAP memory_map = NULL;

    EFI_STATUS Status = get_memory_map(SystemTable, &memory_map);

    EFI_MEMORY_DESCRIPTOR *desc = memory_map->MemoryMap;
    UINTN count = memory_map->MemoryMapSize / memory_map->DescriptorSize;
    UINT64 pages = 0;

    Print(L"MemoryMapSize = %lu\r\n", memory_map->MemoryMapSize);
    Print(L"DescriptorSize = %lu\r\n", memory_map->DescriptorSize);
    Print(L"Count = %lu\r\n", count);

    for (UINTN i = 0; i < count; i++)
    {
        Print(
            L"[%lu] Type=%u Pages=%lu PhysicalStart=0x%lx\r\n",
            i,
            desc->Type,
            desc->NumberOfPages,
            desc->PhysicalStart);

        pages += desc->NumberOfPages;
        desc = (EFI_MEMORY_DESCRIPTOR *)((CHAR8 *)desc + memory_map->DescriptorSize);
    }

    Print(L"Total pages = %lu\r\n", pages);
    Print(L"Total MiB = %lu\r\n", pages / 256);
    */

    PMEMORY_MAP mem_map = NULL;
    EFI_MEMORY_DESCRIPTOR *desc = NULL;

    get_memory_map(SystemTable, &mem_map);

    desc = mem_map->MemoryMap;

    UINTN bytes = 0;
    UINTN pages = 0;

    for (UINTN i = 0; i < mem_map->MemoryMapSize / mem_map->DescriptorSize; i++)
    {

        pages += desc->NumberOfPages;
        bytes += desc->NumberOfPages * 4096;

        desc = (EFI_MEMORY_DESCRIPTOR *)((CHAR8 *)desc + mem_map->DescriptorSize);
    }

    LOG_INFO("Total memory: %s", convert(bytes));
}

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
