#ifndef IMAGE_H
#define IMAGE_H

#include <efi/efi.h>

EFI_STATUS load_image_by_path(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, CHAR16 *path, EFI_HANDLE *ImageHandleOut);
EFI_STATUS start_image(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

#endif
