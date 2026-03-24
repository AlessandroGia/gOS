#include <stdint.h>

#include "arch/x86_64/memory/paging.h"
#include "core/early_allocator.h"
#include "lib/kmemory.h"

PageTable *g_pml4_table = 0;

void paging_load_pml4(PageTable pml4)
{
    if (pml4 == 0)
    {
        return;
    }

    __asm__ __volatile__(
        "mov %0, %%cr3"
        :
        : "r"((uint64_t)pml4)
        : "memory");
}

PageTable paging_alloc_table(void)
{
    PageTable table = (PageTable)early_alloc(PAGE_SIZE, PAGE_SIZE);

    if (table == 0)
    {
        return 0;
    }

    kmemset(table, 0, PAGE_SIZE);
    return table;
}

void paging_set_entry(PageTableEntry *entry, uint64_t physical_address, uint64_t flags)
{
    if (entry == 0)
    {
        return;
    }

    *entry = (physical_address & 0x000FFFFFFFFFF000ULL) | flags;
}

void get_indices_from_virtual_address(uint64_t virtual_address,
                                      uint64_t *pml4_index,
                                      uint64_t *pdpt_index,
                                      uint64_t *pd_index,
                                      uint64_t *pt_index)
{
    if (pml4_index != 0)
    {
        *pml4_index = (virtual_address >> 39) & 0x1FF;
    }

    if (pdpt_index != 0)
    {
        *pdpt_index = (virtual_address >> 30) & 0x1FF;
    }

    if (pd_index != 0)
    {
        *pd_index = (virtual_address >> 21) & 0x1FF;
    }

    if (pt_index != 0)
    {
        *pt_index = (virtual_address >> 12) & 0x1FF;
    }
}

uint64_t paging_get_entry_address(PageTableEntry entry)
{
    return entry & PAGE_ADDRESS_MASK;
}

int paging_entry_present(PageTableEntry entry)
{
    return (entry & PAGE_PRESENT) != 0;
}

PageTableIndices paging_get_indices(uint64_t virtual_address)
{
    PageTableIndices indices;

    indices.pml4 = (uint16_t)((virtual_address >> 39) & 0x1FF);
    indices.pdpt = (uint16_t)((virtual_address >> 30) & 0x1FF);
    indices.pd = (uint16_t)((virtual_address >> 21) & 0x1FF);
    indices.pt = (uint16_t)((virtual_address >> 12) & 0x1FF);

    return indices;
}

int paging_map_page(PageTable pml4,
                    uint64_t virtual_address,
                    uint64_t physical_address,
                    uint64_t flags)
{
    if (pml4 == 0)
    {
        return 0;
    }

    if ((virtual_address & (PAGE_SIZE - 1)) != 0)
    {
        return 0;
    }

    if ((physical_address & (PAGE_SIZE - 1)) != 0)
    {
        return 0;
    }

    PageTableIndices indices = paging_get_indices(virtual_address);

    PageTable pdpt;
    PageTable pd;
    PageTable pt;

    if (!paging_entry_present(pml4[indices.pml4]))
    {
        pdpt = paging_alloc_table();
        if (pdpt == 0)
        {
            return 0;
        }

        paging_set_entry(&pml4[indices.pml4],
                         (uint64_t)pdpt,
                         PAGE_PRESENT | PAGE_WRITABLE);
    }
    else
    {
        pdpt = (PageTable)paging_get_entry_address(pml4[indices.pml4]);
    }

    if (!paging_entry_present(pdpt[indices.pdpt]))
    {
        pd = paging_alloc_table();
        if (pd == 0)
        {
            return 0;
        }

        paging_set_entry(&pdpt[indices.pdpt],
                         (uint64_t)pd,
                         PAGE_PRESENT | PAGE_WRITABLE);
    }
    else
    {
        pd = (PageTable)paging_get_entry_address(pdpt[indices.pdpt]);
    }

    if (!paging_entry_present(pd[indices.pd]))
    {
        pt = paging_alloc_table();
        if (pt == 0)
        {
            return 0;
        }

        paging_set_entry(&pd[indices.pd],
                         (uint64_t)pt,
                         PAGE_PRESENT | PAGE_WRITABLE);
    }
    else
    {
        pt = (PageTable)paging_get_entry_address(pd[indices.pd]);
    }

    paging_set_entry(&pt[indices.pt], physical_address, flags);
    return 1;
}

int paging_map_range_identity(PageTable pml4,
                              uint64_t start_address,
                              uint64_t size,
                              uint64_t flags)
{
    return paging_map_range(pml4, start_address, start_address, size, flags);
}

int paging_map_range(PageTable pml4,
                     uint64_t virtual_start,
                     uint64_t physical_start,
                     uint64_t size,
                     uint64_t flags)
{
    if (pml4 == 0 || size == 0)
    {
        return 0;
    }

    if ((virtual_start & (PAGE_SIZE - 1)) != 0)
    {
        return 0;
    }

    if ((physical_start & (PAGE_SIZE - 1)) != 0)
    {
        return 0;
    }

    uint64_t virtual_end = virtual_start + size;
    if (virtual_end < virtual_start)
    {
        return 0;
    }

    uint64_t current_virtual = virtual_start;
    uint64_t current_physical = physical_start;

    while (current_virtual < virtual_end)
    {
        if (!paging_map_page(pml4, current_virtual, current_physical, flags))
        {
            return 0;
        }

        current_virtual += PAGE_SIZE;
        current_physical += PAGE_SIZE;
    }

    return 1;
}

static uint64_t align_down(uint64_t value, uint64_t alignment)
{
    if (alignment == 0)
    {
        return value;
    }

    return value & ~(alignment - 1);
}

static uint64_t align_up(uint64_t value, uint64_t alignment)
{
    if (alignment == 0)
    {
        return value;
    }

    uint64_t mask = alignment - 1;
    return (value + mask) & ~mask;
}

int paging_map_region_identity(PageTable pml4,
                               MemoryRegionInfo region,
                               uint64_t flags)
{
    if (region.base == 0 || region.size == 0)
    {
        return 0;
    }

    uint64_t start = align_down((uint64_t)region.base, PAGE_SIZE);
    uint64_t end = align_up((uint64_t)region.base + region.size, PAGE_SIZE);

    if (end < start)
    {
        return 0;
    }

    return paging_map_range_identity(pml4, start, end - start, flags);
}
