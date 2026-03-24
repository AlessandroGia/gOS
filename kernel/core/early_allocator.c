#include <stdint.h>

#include "core/early_allocator.h"
#include "core/memory/memory.h"

static uint64_t g_early_base = 0;
static uint64_t g_early_current = 0;
static uint64_t g_early_end = 0;

static const MemoryRegionInfo *g_reserved_regions = 0;
static uint64_t g_reserved_region_count = 0;

static uint64_t align_up(uint64_t value, uint64_t alignment)
{
    if (alignment == 0)
    {
        return value;
    }

    uint64_t mask = alignment - 1;
    return (value + mask) & ~mask;
}

static uint64_t region_end(uint64_t base, uint64_t size)
{
    uint64_t end = base + size;

    if (end < base)
    {
        return 0;
    }

    return end;
}

static int allocation_overlaps_reserved(uint64_t base, uint64_t size, uint64_t *overlap_end)
{
    for (uint64_t i = 0; i < g_reserved_region_count; i++)
    {
        uint64_t reserved_base = (uint64_t)g_reserved_regions[i].base;
        uint64_t reserved_size = g_reserved_regions[i].size;

        if (memory_region_overlaps(base, size, reserved_base, reserved_size))
        {
            if (overlap_end != 0)
            {
                *overlap_end = region_end(reserved_base, reserved_size);
            }

            return 1;
        }
    }

    return 0;
}

void early_allocator_init(const MemoryInfo *memory_info,
                          const ReservedRegionsInfo *reserved_info)
{
    g_early_base = 0;
    g_early_current = 0;
    g_early_end = 0;

    g_reserved_regions = 0;
    g_reserved_region_count = 0;

    if (reserved_info != 0)
    {
        g_reserved_regions = reserved_info->regions;
        g_reserved_region_count = reserved_info->count;
    }

    if (memory_info == 0)
    {
        return;
    }

    if (memory_info->largest_usable_region_base == 0 ||
        memory_info->largest_usable_region_size == 0)
    {
        return;
    }

    g_early_base = memory_info->largest_usable_region_base;
    g_early_current = g_early_base;
    g_early_end = g_early_base + memory_info->largest_usable_region_size;
}

void *early_alloc(uint64_t size, uint64_t alignment)
{
    if (size == 0)
    {
        return 0;
    }

    if (g_early_current == 0 || g_early_end == 0)
    {
        return 0;
    }

    uint64_t candidate = g_early_current;

    for (;;)
    {
        candidate = align_up(candidate, alignment);
        uint64_t next = candidate + size;

        if (next > g_early_end || next < candidate)
        {
            return 0;
        }

        uint64_t overlap_end = 0;
        if (!allocation_overlaps_reserved(candidate, size, &overlap_end))
        {
            g_early_current = next;
            return (void *)candidate;
        }

        if (overlap_end == 0 || overlap_end <= candidate)
        {
            return 0;
        }

        candidate = overlap_end;
    }
}

uint64_t early_allocator_remaining(void)
{
    if (g_early_current >= g_early_end)
    {
        return 0;
    }

    return g_early_end - g_early_current;
}

uint64_t early_allocator_base(void)
{
    return g_early_base;
}

uint64_t early_allocator_current(void)
{
    return g_early_current;
}
