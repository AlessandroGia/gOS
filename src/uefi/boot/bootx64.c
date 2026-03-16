#include <efi/efi.h>
#include <efi/efilib.h>
#include <efi/efiprot.h>

#include "shared/bootinfo.h"

#include "uefi/common/console/menu/menu.h"

#include "uefi/boot/menu/actions.h"
#include "uefi/common/memory/memory.h"

#define BOOT_MENU_NAME L"gOS Boot Menu"

typedef EFI_STATUS EFIAPI EFIMAIN;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    MenuEntry *menu_entries = get_boot_menu_entries();
    UINTN menu_entry_count = get_boot_menu_entry_count();

    InitializeLib(ImageHandle, SystemTable);

    MemoryLeakContext mem_leak_ctx = {0};
    check_memory_leak(SystemTable, &mem_leak_ctx, FALSE);
    menu_entries[1].context = &mem_leak_ctx;

    Status = run_menu(
        ImageHandle,
        SystemTable,
        BOOT_MENU_NAME,
        menu_entries,
        menu_entry_count,
        FALSE,
        20);

    return Status;
}
