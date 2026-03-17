#ifndef INPUT_H
#define INPUT_H

#include <efi/efi.h>

EFI_INPUT_KEY get_key(EFI_SYSTEM_TABLE *SystemTable);
void wait_for_enter(EFI_SYSTEM_TABLE *SystemTable);

#endif
