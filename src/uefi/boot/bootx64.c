#include <efi/efi.h>
#include <efi/efilib.h>
#include <efi/efiprot.h>

#include "shared/bootinfo.h"

#include "uefi/common/console/screen.h"
#include "uefi/common/console/input.h"
#include "uefi/common/memory/memory.h"
#include "uefi/common/image/image.h"

#include "uefi/boot/menu/menu.h"

typedef EFI_STATUS EFIAPI EFIMAIN;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    UINTN selected = 0;

    InitializeLib(ImageHandle, SystemTable);

    Status = run_menu(
        ImageHandle,
        SystemTable,
        (const CHAR16 *)L"gOS Boot Menu",
        get_boot_menu_entries(),
        get_boot_menu_entry_count(),
        &selected);
    if (EFI_ERROR(Status))
        return Status;

    return execute_menu_entry(
        &menu_entries[selected],
        ImageHandle,
        SystemTable);
}
