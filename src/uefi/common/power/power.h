#ifndef POWER_H
#define POWER_H

#include <efi/efi.h>

void shutdown(EFI_SYSTEM_TABLE *SystemTable);
void reboot(EFI_SYSTEM_TABLE *SystemTable);

#endif
