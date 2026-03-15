#include <efi/efi.h>

#include "power.h"

static void reset_system(EFI_SYSTEM_TABLE *SystemTable, EFI_RESET_TYPE ResetType)
{
    uefi_call_wrapper(SystemTable->RuntimeServices->ResetSystem, 4, ResetType, EFI_SUCCESS, 0, NULL);
}

void shutdown(EFI_SYSTEM_TABLE *SystemTable)
{
    reset_system(SystemTable, EfiResetShutdown);
}

void reboot(EFI_SYSTEM_TABLE *SystemTable)
{
    reset_system(SystemTable, EfiResetCold);
}
