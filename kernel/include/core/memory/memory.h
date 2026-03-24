#ifndef KERNEL_CORE_MEMORY_H
#define KERNEL_CORE_MEMORY_H

#define UEFI_CONVENTIONAL_MEMORY_TYPE 7
#define PAGE_SIZE 4096ULL

#include <stdint.h>
#include <shared/bootinfo.h>

typedef struct
{
    uint64_t total_memory_bytes;
    uint64_t usable_memory_bytes;
    uint64_t largest_usable_region_base;
    uint64_t largest_usable_region_size;
} MemoryInfo;

typedef struct
{
    const MemoryRegionInfo *regions;
    uint64_t count;
} ReservedRegionsInfo;

void memory_init(const MemoryMapInfo *memory_map);
const MemoryInfo *memory_get_info(void);

int memory_region_overlaps(uint64_t base_a, uint64_t size_a,
                           uint64_t base_b, uint64_t size_b);

#endif
