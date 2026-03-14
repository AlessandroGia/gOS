#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <efi/efi.h>

#include "shared/bootinfo.h"

EFI_STATUS get_framebuffer_info(
    EFI_SYSTEM_TABLE *st,
    BootInfo *boot_info);

#endif
