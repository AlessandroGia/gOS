#include <efi/efi.h>

#include "uefi/common/console/input.h"

EFI_INPUT_KEY get_key(EFI_SYSTEM_TABLE *SystemTable)
{

    EFI_INPUT_KEY key;
    UINTN index;

    uefi_call_wrapper(
        SystemTable->BootServices->WaitForEvent, 3,
        1,
        &SystemTable->ConIn->WaitForKey,
        &index);

    uefi_call_wrapper(
        SystemTable->ConIn->ReadKeyStroke, 2,
        SystemTable->ConIn,
        &key);

    return key;
}

void wait_for_enter(EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_INPUT_KEY key;

    for (;;)
    {
        key = get_key(SystemTable);

        if (key.UnicodeChar == CHAR_CARRIAGE_RETURN)
            break;
    }
}
