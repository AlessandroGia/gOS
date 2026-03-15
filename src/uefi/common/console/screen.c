#include <efi/efi.h>

#include "screen.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/console/input.h"
#include "uefi/common/log/log.h"

void clear_screen(EFI_SYSTEM_TABLE *SystemTable)
{
    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
}

void press_enter_to_continue(EFI_SYSTEM_TABLE *SystemTable)
{
    Print(L"\r\nPress Enter to continue...");
    wait_for_enter(SystemTable);
}
