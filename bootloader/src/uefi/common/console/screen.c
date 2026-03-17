#include <efi/efi.h>

#include "uefi/common/console/screen.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/console/input.h"
#include "uefi/common/log/log.h"

EFI_STATUS clear_screen(EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status = EFI_SUCCESS;
    status = uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    if (EFI_ERROR(status))
    {
        log_error(L"Failed to clear screen: %r", status);
        return status;
    }
    return EFI_SUCCESS;
}

void press_enter_to_continue(EFI_SYSTEM_TABLE *SystemTable)
{
    Print(L"\r\nPress Enter to continue...");
    wait_for_enter(SystemTable);
}
