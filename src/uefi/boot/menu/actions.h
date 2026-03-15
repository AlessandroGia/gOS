#ifndef ACTIONS_H
#define ACTIONS_H

#include <efi/efi.h>

/* Menu action callbacks */

EFI_STATUS action_load_and_start_image(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context);

EFI_STATUS action_exit(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context);

EFI_STATUS action_reboot(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context);

EFI_STATUS action_shutdown(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context);

#endif
