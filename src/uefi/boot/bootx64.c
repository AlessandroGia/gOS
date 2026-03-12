
#include <efi/efi.h>
#include <efi/efilib.h>

void clear_screen(EFI_SYSTEM_TABLE *SystemTable)
{
    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
}

void sleep(EFI_SYSTEM_TABLE *SystemTable, UINT32 sec)
{
    uefi_call_wrapper(SystemTable->BootServices->Stall, 1, sec * 1000000);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    clear_screen(SystemTable);
    Print(L"Hello, gOS!\n");
    sleep(SystemTable, 3);

    return EFI_SUCCESS;
}
