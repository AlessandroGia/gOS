#ifndef STRUCTS_H
#define STRUCTS_H

#include <efi/efi.h>

typedef struct memory_map
{
    UINTN MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} MEMORY_MAP;

typedef MEMORY_MAP *PMEMORY_MAP;

EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *SystemTable, PMEMORY_MAP *mem_map);
void prov(EFI_SYSTEM_TABLE *SystemTable);

#endif
