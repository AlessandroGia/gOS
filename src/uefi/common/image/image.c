#include <efi/efi.h>
#include <efi/efilib.h>

#include "image.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/image/image.h"
#include "uefi/common/log/log.h"

EFI_STATUS load_image_by_path(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, CHAR16 *path, EFI_HANDLE *ImageHandleOut)
{
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_DEVICE_PATH_PROTOCOL *ImagePath = NULL;

    Status = uefi_call_wrapper(
        SystemTable->BootServices->OpenProtocol, 6,
        ImageHandle,
        &LoadedImageProtocol,
        (VOID **)&LoadedImage,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to open LoadedImage protocol: %r", Status);
        goto cleanup;
    }

    ImagePath = FileDevicePath(LoadedImage->DeviceHandle, path);
    if (ImagePath == NULL)
    {
        LOG_ERROR("Failed to create device path for OS loader.");
        goto cleanup;
    }

    Status = uefi_call_wrapper(
        SystemTable->BootServices->LoadImage, 6,
        FALSE,
        ImageHandle,
        ImagePath,
        NULL,
        0,
        ImageHandleOut);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to load OS loader image: %r", Status);
        goto cleanup;
    }

    return EFI_SUCCESS;

cleanup:
    if (ImagePath != NULL)
        FreePool(ImagePath);
    return Status;
}

EFI_STATUS start_image(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;

    Status = uefi_call_wrapper(
        SystemTable->BootServices->StartImage, 3,
        ImageHandle,
        NULL,
        NULL);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to start image: %r", Status);
        return Status;
    }

    return EFI_SUCCESS;
}
