#include <efi/efi.h>
#include <efi/efilib.h>

#include "memory.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

CHAR16 *convert_bytes_to_string(UINTN byte)
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

UINTN get_total_memory_by_type(PMEMORY_MAP mem_map, EFI_MEMORY_TYPE type)
{
    EFI_MEMORY_DESCRIPTOR *desc = NULL;
    UINTN bytes = 0;
    UINTN count = mem_map->MemoryMapSize / mem_map->DescriptorSize;

    desc = mem_map->MemoryMap;

    for (UINTN i = 0; i < count; i++)
    {
        if (desc->Type == type)
            bytes += desc->NumberOfPages * 4096;

        desc = (EFI_MEMORY_DESCRIPTOR *)((CHAR8 *)desc + mem_map->DescriptorSize);
    }

    return bytes;
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

void free_mem_map(EFI_SYSTEM_TABLE *SystemTable, PMEMORY_MAP *mem_map)
{
    if (mem_map != NULL && *mem_map != NULL)
    {
        if ((*mem_map)->MemoryMap != NULL)
        {
            free_pool(SystemTable, (*mem_map)->MemoryMap);
            (*mem_map)->MemoryMap = NULL;
        }

        free_pool(SystemTable, *mem_map);
        *mem_map = NULL;
    }
}

void check_memory_leak(EFI_SYSTEM_TABLE *SystemTable, MemoryLeakContext *ctx, BOOLEAN probe)
{
    EFI_STATUS Status;
    PMEMORY_MAP mem_map = NULL;
    UINTN bytes = 0;

    if (SystemTable == NULL || ctx == NULL)
        return;

    Status = get_memory_map(SystemTable, &mem_map);
    if (EFI_ERROR(Status) || mem_map == NULL)
        return;

    bytes = get_total_memory_by_type(mem_map, EfiConventionalMemory);
    free_mem_map(SystemTable, &mem_map);

    if (ctx->baseline_bytes == 0)
    {
        ctx->baseline_bytes = bytes;
        ctx->prev_bytes = bytes;
        return;
    }

    if (probe)
    {
        INTN delta_prev = (INTN)bytes - (INTN)ctx->prev_bytes;
        INTN delta_baseline = (INTN)bytes - (INTN)ctx->baseline_bytes;

        CHAR16 *baseline_str = convert_bytes_to_string(ctx->baseline_bytes);
        CHAR16 *prev_str = convert_bytes_to_string(ctx->prev_bytes);
        CHAR16 *bytes_str = convert_bytes_to_string(bytes);
        CHAR16 *delta_prev_str = convert_bytes_to_string(delta_prev < 0 ? (UINTN)(-delta_prev) : (UINTN)delta_prev);
        CHAR16 *delta_baseline_str = convert_bytes_to_string(delta_baseline < 0 ? (UINTN)(-delta_baseline) : (UINTN)delta_baseline);

        Print(
            L"Memory Leak Check\r\n\n\n"
            L"Baseline: %s\r\n"
            L"Previous: %s\r\n"
            L"Current : %s\r\n\n"
            L"Delta (Previous): %s%s\r\n"
            L"Delta (Baseline): %s%s\r\n\n",
            baseline_str ? baseline_str : L"(null)",
            prev_str ? prev_str : L"(null)",
            bytes_str ? bytes_str : L"(null)",
            delta_prev < 0 ? L"-" : L"+",
            delta_prev_str ? delta_prev_str : L"(null)",
            delta_baseline < 0 ? L"-" : L"+",
            delta_baseline_str ? delta_baseline_str : L"(null)");

        free_pool(SystemTable, baseline_str);
        free_pool(SystemTable, prev_str);
        free_pool(SystemTable, bytes_str);
        free_pool(SystemTable, delta_prev_str);
        free_pool(SystemTable, delta_baseline_str);
    }

    ctx->prev_bytes = bytes;
}
