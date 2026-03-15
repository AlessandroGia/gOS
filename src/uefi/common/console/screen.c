#include <efi/efi.h>

#include "screen.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

void clear_screen(EFI_SYSTEM_TABLE *SystemTable)
{
    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
}
