#include <efi/efi.h>

#include "actions.h"
#include "menu.h"

#include "uefi/common/image/image.h"
#include "uefi/common/power/power.h"

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
