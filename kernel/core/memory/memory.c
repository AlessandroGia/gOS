#include <stdint.h>

#include "shared/uefi_memory.h"

#include "core/memory/memory.h"

static MemoryInfo g_memory_info = {0};

void memory_init(const MemoryMapInfo *memory_map)
{

    g_memory_info.total_memory_bytes = 0;
    g_memory_info.usable_memory_bytes = 0;
    g_memory_info.largest_usable_region_base = 0;
    g_memory_info.largest_usable_region_size = 0;

    if (memory_map == 0 || memory_map->base == 0 || memory_map->descriptor_size == 0)
    {
        return;
    }

    uint64_t entry_count = memory_map->size / memory_map->descriptor_size;

    for (uint64_t i = 0; i < entry_count; i++)
    {
        UefiMemoryDescriptor *desc = (UefiMemoryDescriptor *)((uint8_t *)memory_map->base + i * memory_map->descriptor_size);

        uint64_t region_size_bytes = desc->number_of_pages * PAGE_SIZE;
        g_memory_info.total_memory_bytes += region_size_bytes;

        if (desc->type == UEFI_CONVENTIONAL_MEMORY_TYPE)
        {
            g_memory_info.usable_memory_bytes += region_size_bytes;

            if (region_size_bytes > g_memory_info.largest_usable_region_size)
            {
                g_memory_info.largest_usable_region_base = (uint64_t)desc->physical_start;
                g_memory_info.largest_usable_region_size = region_size_bytes;
            }
        }
    }
}

int memory_region_overlaps(uint64_t base_a, uint64_t size_a,
                           uint64_t base_b, uint64_t size_b)
{
    uint64_t end_a = base_a + size_a;
    uint64_t end_b = base_b + size_b;

    if (end_a < base_a || end_b < base_b)
    {
        return 0;
    }

    return (base_a < end_b) && (base_b < end_a);
}

const MemoryInfo *memory_get_info(void)
{
    return &g_memory_info;
}
