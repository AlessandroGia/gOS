#include <efi/efi.h>
#include <efi/efilib.h>

#include "memory_menu.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/memory/memory.h"
#include "uefi/common/log/log.h"
#include "uefi/common/console/input.h"

#include "uefi/common/console/menu/menu.h"

static MenuEntry menu_entries[] = {
    {L"Show Memory Map", action_show_memory_map, NULL},
    {L"Check Memory Leaks", action_check_memory_leak, NULL},
}; // CHECKMEMORYLEAKS must be in index 1 for context passing

void show_memory_menu(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, MemoryLeakContext *mem_leak_ctx)
{
    menu_entries[1].context = mem_leak_ctx;
    run_menu(ImageHandle, SystemTable, L"Memory", menu_entries, sizeof(menu_entries) / sizeof(menu_entries[0]), TRUE, 20);
}

EFI_STATUS action_check_memory_leak(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context)
{
    MemoryLeakContext *ctx = (MemoryLeakContext *)context;
    check_memory_leak(SystemTable, ctx, TRUE);
    Print(L"\r\nPress Enter to continue...");
    wait_for_enter(SystemTable);

    return EFI_SUCCESS;
}

EFI_STATUS action_show_memory_map(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context)
{
    (VOID) context;

    PMEMORY_MAP mem_map = NULL;
    EFI_MEMORY_DESCRIPTOR *desc = NULL;

    get_memory_map(SystemTable, &mem_map);

    desc = mem_map->MemoryMap;

    UINTN bytes = 0;
    UINTN pages = 0;

    UINTN count = mem_map->MemoryMapSize / mem_map->DescriptorSize;
    UINTN free_space = 0;

    MenuEntry entries[count];

    for (UINTN i = 0; i < count; i++)
    {

        pages += desc->NumberOfPages;
        bytes += desc->NumberOfPages * 4096;

        if (desc->Type == EfiConventionalMemory)
            free_space += desc->NumberOfPages * 4096;

        entries[i].label = PoolPrint(
            L"[%d] Type=%u Pages=%lu PhysicalStart=0x%lx",
            i + 1,
            desc->Type,
            desc->NumberOfPages,
            desc->PhysicalStart);
        entries[i].action = NULL;
        entries[i].context = NULL;

        desc = (EFI_MEMORY_DESCRIPTOR *)((CHAR8 *)desc + mem_map->DescriptorSize);
    }

    CHAR16 *free_space_str = convert_bytes_to_string(free_space);
    CHAR16 *title = PoolPrint(L"Memory Map - Free: %s", free_space_str);

    run_menu(ImageHandle, SystemTable, title, entries, count, TRUE, 20);

    free_pool(SystemTable, mem_map->MemoryMap);
    free_pool(SystemTable, mem_map);
    free_pool(SystemTable, title);
    free_pool(SystemTable, free_space_str);
    for (UINTN i = 0; i < count; i++)
        free_pool(SystemTable, (VOID *)entries[i].label);
}
