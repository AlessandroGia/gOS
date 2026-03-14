#include <efi/efi.h>
#include <efi/efilib.h>
#include <efi/efiprot.h>

#include "shared/bootinfo.h"
#include "uefi/common/memory/memory.h"
#include "uefi/common/input/input.h"

typedef EFI_STATUS EFIAPI EFIMAIN;

static EFI_STATUS load_image_osloader(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_DEVICE_PATH_PROTOCOL *OsLoaderPath = NULL;
    EFI_HANDLE OsLoaderHandle = NULL;

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
        Print(L"Failed to open LoadedImage protocol: %r\r\n", Status);
        return Status;
    }

    OsLoaderPath = FileDevicePath(LoadedImage->DeviceHandle, L"\\EFI\\BOOT\\OsLoader.efi");
    if (OsLoaderPath == NULL)
    {
        Print(L"FileDevicePath() failed.\r\n");
        return EFI_OUT_OF_RESOURCES;
    }

    Status = uefi_call_wrapper(
        SystemTable->BootServices->LoadImage, 6,
        FALSE,
        ImageHandle,
        OsLoaderPath,
        NULL,
        0,
        &OsLoaderHandle);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to load OS loader image: %r\r\n", Status);
        return Status;
    }

    Status = uefi_call_wrapper(
        SystemTable->BootServices->StartImage, 3,
        OsLoaderHandle,
        NULL,
        NULL);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to start OS loader image: %r\r\n", Status);
        return Status;
    }

    return EFI_SUCCESS;
}

void clear_screen(EFI_SYSTEM_TABLE *SystemTable)
{
    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
}

void print_menu()
{
    Print(L"gOS Bootloader\n");
    Print(L"1. Load OS Loader\n");
    Print(L"2. Exit\n");
}

EFIMAIN efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

    EFI_INPUT_KEY key;

    PMEMORY_MAP mem_map = get_memory_map(SystemTable);

    InitializeLib(ImageHandle, SystemTable);
    clear_screen(SystemTable);

    do
    {
        clear_screen(SystemTable);
        print_menu(SystemTable);
        key = get_key(SystemTable);

        switch (key.UnicodeChar)
        {
        case L'1':
            load_image_osloader(ImageHandle, SystemTable);
            break;
        case L'2':
            uefi_call_wrapper(SystemTable->BootServices->Exit, 0);
            break;
        default:
            Print(L"Invalid option. Please try again.\n");
            break;
        }

        Print(L"Press any key to continue...\n");
        get_key(SystemTable);

    } while (key.UnicodeChar != L'2');

    return EFI_SUCCESS;
}
