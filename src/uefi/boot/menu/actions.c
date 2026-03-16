#include <efi/efi.h>
#include <efi/efilib.h>

#include "actions.h"

#include "uefi/common/console/menu/menu.h"
#include "uefi/common/console/screen.h"
#include "uefi/common/image/image.h"
#include "uefi/common/power/power.h"
#include "uefi/common/memory/memory.h"
#include "uefi/memory_menu/memory_menu.h"

static MenuEntry menu_entries[] =
    {
        {L"Load Kernel", action_load_and_start_image, L"\\EFI\\BOOT\\OsLoader.efi"},
        {L"Memory", action_memory_map, NULL},
        {L"Firmware", action_exit, NULL},
        {L"Reboot", action_reboot, NULL},
        {L"Shutdown", action_shutdown, NULL}}; // MEMORY must be in index 1 for context passing

MenuEntry *get_boot_menu_entries(void)
{
    return menu_entries;
}

UINTN get_boot_menu_entry_count(void)
{
    return sizeof(menu_entries) / sizeof(menu_entries[0]);
}

EFI_STATUS action_memory_map(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context)
{
    MemoryLeakContext *mem_leak_ctx = (MemoryLeakContext *)context;

    show_memory_menu(ImageHandle, SystemTable, mem_leak_ctx);

    return EFI_SUCCESS;
}

EFI_STATUS action_load_and_start_image(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context)
{
    CHAR16 *Path = (CHAR16 *)context;
    EFI_HANDLE LoadedImageHandle = NULL;

    load_image_by_path(ImageHandle, SystemTable, Path, &LoadedImageHandle);
    return start_image(LoadedImageHandle, SystemTable);
}

EFI_STATUS action_exit(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context)
{
    (VOID) context;

    uefi_call_wrapper(SystemTable->BootServices->Exit, 4, ImageHandle, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}

EFI_STATUS action_reboot(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context)
{
    (VOID) ImageHandle;
    (VOID) context;

    reboot(SystemTable);
    return EFI_SUCCESS;
}

EFI_STATUS action_shutdown(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context)
{
    (VOID) ImageHandle;
    (VOID) context;

    shutdown(SystemTable);
    return EFI_SUCCESS;
}
