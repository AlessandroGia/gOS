#ifndef HELPER_H
#define HELPER_H

#include <efi/efi.h>

EFI_STATUS free_pool(EFI_SYSTEM_TABLE *SystemTable, VOID *ptr);

#endif
