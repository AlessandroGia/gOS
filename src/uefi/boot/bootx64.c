
#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLibrary(ImageHandle, SystemTable);
    Print(L"Hello, gOS!\n");
    return EFI_SUCCESS;
}
