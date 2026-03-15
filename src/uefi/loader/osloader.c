#include <efi/efi.h>
#include <efi/efilib.h>
#include <efi/efiprot.h>

#include "shared/bootinfo.h"

#include "uefi/common/memory/memory.h"
#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

#include "uefi/loader/framebuffer/framebuffer.h"
#include "uefi/loader/kernel/bin/kernel.h"
#include "uefi/loader/handoff/handoff.h"

#define KERNEL_LOAD_ADDRESS 0x100000

typedef EFI_STATUS EFIAPI EFIMAIN;
typedef void (*KernelEntry)(BootInfo *boot_info);

EFIMAIN efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    VOID *KernelBuffer = NULL;
    UINTN KernelSize = 0;
    PMEMORY_MAP mem_map = NULL;
    BootInfo BootInfoData = {0};

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

    copy_kernel_to_address(KernelBuffer, KernelSize, (VOID *)KERNEL_LOAD_ADDRESS);
    LOG_INFO(L"Kernel copied to 0x100000.");

    if (EFI_ERROR(get_memory_map(SystemTable, &mem_map)))
    {
        LOG_ERROR("Memory map retrieval failed.");
        goto cleanup;
    }

    exit_boot_services_with_retry(ImageHandle, SystemTable, &mem_map, &BootInfoData);
    jump_to_kernel((VOID *)KERNEL_LOAD_ADDRESS, &BootInfoData);

    for (;;)
    {
        __asm__ __volatile__("hlt");
    }

    return EFI_SUCCESS;

cleanup:
    if (mem_map)
    {
        if (mem_map->MemoryMap)
        {
            free_pool(SystemTable, mem_map->MemoryMap);
            mem_map->MemoryMap = NULL;
        }
        free_pool(SystemTable, mem_map);
    }
    if (KernelBuffer)
        free_pool(SystemTable, KernelBuffer);

    return Status;
}
