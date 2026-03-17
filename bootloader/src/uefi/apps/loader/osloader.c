#include <efi/efi.h>
#include <efi/efilib.h>
#include <efi/efiprot.h>

#include "shared/bootinfo.h"

#include "uefi/common/memory/memory.h"
#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

#include "uefi/apps/loader/framebuffer/framebuffer.h"
#include "uefi/apps/loader/kernel/bin/kernel.h"
#include "uefi/apps/loader/handoff/handoff.h"

#define KERNEL_LOAD_ADDRESS 0x100000

typedef EFI_STATUS EFIAPI EFIMAIN;
typedef void (*KernelEntry)(BootInfo *boot_info);

EFIMAIN efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    VOID *KernelBuffer = NULL;
    UINTN KernelSize = 0;
    BootInfo BootInfoData = {0};
    EFI_PHYSICAL_ADDRESS KernelDestination = KERNEL_LOAD_ADDRESS;

    InitializeLib(ImageHandle, SystemTable);

    Status = get_framebuffer_info(SystemTable, &BootInfoData);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Framebuffer setup failed.");
        goto cleanup;
    }
    LOG_INFO(
        L"GOP ok: %ux%u",
        BootInfoData.framebuffer_width,
        BootInfoData.framebuffer_height);

    Status = load_kernel_file(ImageHandle, SystemTable, &KernelBuffer, &KernelSize);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Kernel load failed.");
        goto cleanup;
    }
    LOG_INFO(L"Kernel loaded successfully. Size: %d bytes", KernelSize);

    Status = load_kernel_to_address(SystemTable, &KernelDestination, KernelSize, KernelBuffer);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to load kernel to address: %r", Status);
        goto cleanup;
    }
    LOG_INFO(L"Kernel copied to 0x%lx.", (UINTN)KernelDestination);

    exit_boot_services_with_retry(ImageHandle, SystemTable, &BootInfoData);
    jump_to_kernel(KernelDestination, &BootInfoData);

    for (;;)
    {
        __asm__ __volatile__("hlt");
    }

    return EFI_SUCCESS;

cleanup:
    if (KernelBuffer)
        free_pool(SystemTable, KernelBuffer);

    return Status;
}
