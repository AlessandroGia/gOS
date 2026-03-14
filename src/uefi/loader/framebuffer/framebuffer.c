#include <efi/efi.h>

#include "shared/bootinfo.h"

EFI_STATUS get_framebuffer_info(
    EFI_SYSTEM_TABLE *st,
    BootInfo *boot_info)
{
    EFI_STATUS Status;
    EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop = NULL;

    Status = uefi_call_wrapper(
        st->BootServices->LocateProtocol,
        3,
        &GopGuid,
        NULL,
        (VOID **)&Gop);

    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to locate GOP: %r", Status);
        return Status;
    }

    boot_info->framebuffer_base =
        (VOID *)Gop->Mode->FrameBufferBase;

    boot_info->framebuffer_width =
        Gop->Mode->Info->HorizontalResolution;

    boot_info->framebuffer_height =
        Gop->Mode->Info->VerticalResolution;

    boot_info->framebuffer_pixels_per_scanline =
        Gop->Mode->Info->PixelsPerScanLine;

    return EFI_SUCCESS;
}
