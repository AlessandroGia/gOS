#include <efi/efi.h>

#include "uefi/common/helper/helper.h"

#include "uefi/common/log/log.h"

EFI_STATUS free_pool(EFI_SYSTEM_TABLE *SystemTable, VOID *ptr)
{
    if (ptr != NULL)
    {
        EFI_STATUS Status = uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, ptr);
        if (EFI_ERROR(Status))
        {
            LOG_ERROR("Failed to free pool: %r", Status);
            return Status;
        }
    }
    return EFI_SUCCESS;
}
