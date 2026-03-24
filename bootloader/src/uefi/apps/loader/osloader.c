#include <efi/efi.h>
#include <efi/efilib.h>
#include <efi/efiprot.h>

#include "shared/bootinfo.h"
#include "shared/gc_kernel_format.h"

#include "uefi/common/memory/memory.h"
#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

#include "uefi/apps/loader/framebuffer/framebuffer.h"
#include "uefi/apps/loader/kernel/bin/kernel.h"
#include "uefi/apps/loader/handoff/handoff.h"

typedef EFI_STATUS EFIAPI EFIMAIN;
typedef void (*KernelEntry)(BootInfo *boot_info);

EFIMAIN efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    BootInfo boot_info_data = {0};
    VOID *kernel_file_buffer = NULL;
    VOID *kernel_raw_buffer = NULL;
    EFI_PHYSICAL_ADDRESS kernel_destination = 0;
    UINTN kernel_size = 0;
    HeaderGC *hdr;

    InitializeLib(ImageHandle, SystemTable);

    status = get_framebuffer_info(SystemTable, &boot_info_data);
    if (EFI_ERROR(status))
    {
        LOG_ERROR(L"[loader] Framebuffer setup failed.");
        goto cleanup;
    }
    LOG_INFO(
        L"[loader] GOP ok: %ux%u",
        boot_info_data.framebuffer.width,
        boot_info_data.framebuffer.height);

    status = load_kernel_file(ImageHandle, SystemTable, &kernel_file_buffer, &kernel_size);
    if (EFI_ERROR(status))
    {
        LOG_ERROR(L"[loader] Kernel load failed.");
        goto cleanup;
    }
    hdr = (HeaderGC *)kernel_file_buffer;

    LOG_INFO(L"[loader] Kernel file loaded successfully. Size: %d bytes", kernel_size);

    status = validate_kernel_header(hdr, kernel_size);
    if (EFI_ERROR(status))
    {
        LOG_ERROR(L"[loader] Kernel header validation failed.");
        goto cleanup;
    }

    LOG_INFO(L"[loader] Kernel header, Size: %d bytes; Payload raw, Size: %d bytes, Memory Size: %d bytes", hdr->header_size, hdr->payload_file_size, hdr->kernel_memory_size);

    kernel_destination = hdr->kernel_address + hdr->entry_point_offset;
    kernel_raw_buffer = (CHAR8 *)kernel_file_buffer + hdr->header_size;

    status = load_kernel_to_address(SystemTable, &kernel_destination, hdr->payload_file_size, hdr->kernel_memory_size, kernel_raw_buffer);
    Print(L"a");
    if (EFI_ERROR(status))
    {
        LOG_ERROR(L"[loader] Failed to load kernel to address: %r", status);
        goto cleanup;
    }
    LOG_INFO(L"[loader] Kernel copied to 0x%lx.", (UINTN)kernel_destination);
    LOG_INFO(L"[loader] Kernel entry point: 0x%lx", (UINTN)kernel_destination);

    status = exit_boot_services_with_retry(ImageHandle, SystemTable, &boot_info_data);
    if (EFI_ERROR(status))
    {
        LOG_ERROR(L"[loader] Failed to exit boot services: %r", status);
        goto cleanup;
    }

    boot_info_data.kernel_image_region.base = (void *)kernel_destination;
    boot_info_data.kernel_image_region.size = hdr->kernel_memory_size;

    boot_info_data.boot_info_region.base = &boot_info_data;
    boot_info_data.boot_info_region.size = sizeof(boot_info_data);

    boot_info_data.memory_map_region.base = boot_info_data.memory_map.base;
    boot_info_data.memory_map_region.size = boot_info_data.memory_map.size;

    boot_info_data.framebuffer_region.base = boot_info_data.framebuffer.base;
    boot_info_data.framebuffer_region.size = (uint64_t)boot_info_data.framebuffer.pixels_per_scanline *
                                             (uint64_t)boot_info_data.framebuffer.height *
                                             sizeof(uint32_t);

    jump_to_kernel(kernel_destination, &boot_info_data);

    for (;;)
    {
        __asm__ __volatile__("hlt");
    }

    return EFI_SUCCESS;

cleanup:
    if (kernel_file_buffer)
        free_pool(SystemTable, kernel_file_buffer);
    if (kernel_raw_buffer)
        free_pool(SystemTable, kernel_raw_buffer);

    return status;
}
