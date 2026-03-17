#ifndef STRUCTS_H
#define STRUCTS_H

#include <efi/efi.h>

typedef struct
{
    UINTN baseline_bytes;
    UINTN prev_bytes;

} MemoryLeakContext;

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
void check_memory_leak(EFI_SYSTEM_TABLE *SystemTable, MemoryLeakContext *ctx, BOOLEAN probe);
EFI_STATUS free_mem_map(EFI_SYSTEM_TABLE *SystemTable, PMEMORY_MAP *mem_map);
UINTN get_total_memory_by_type(PMEMORY_MAP mem_map, EFI_MEMORY_TYPE type);

CHAR16 *convert_bytes_to_string(UINTN byte);

#endif
