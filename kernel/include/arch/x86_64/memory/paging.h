#ifndef KERNEL_ARCH_X86_64_PAGING_H
#define KERNEL_ARCH_X86_64_PAGING_H

#include <stdint.h>
#include "core/memory/memory.h"

#define PAGE_SIZE 4096ULL
#define PAGE_TABLE_ENTRIES 512
#define PAGE_ADDRESS_MASK 0x000FFFFFFFFFF000ULL

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_WRITE_THROUGH (1ULL << 3)
#define PAGE_CACHE_DISABLE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_NO_EXECUTE (1ULL << 63)

typedef struct
{
    uint16_t pml4;
    uint16_t pdpt;
    uint16_t pd;
    uint16_t pt;
} PageTableIndices;

typedef uint64_t PageTableEntry;
typedef PageTableEntry *PageTable;

PageTable paging_alloc_table(void);

void paging_set_entry(PageTableEntry *entry, uint64_t physical_address, uint64_t flags);
uint64_t paging_get_entry_address(PageTableEntry entry);
int paging_entry_present(PageTableEntry entry);

PageTableIndices paging_get_indices(uint64_t virtual_address);

int paging_map_page(PageTable pml4,
                    uint64_t virtual_address,
                    uint64_t physical_address,
                    uint64_t flags);

int paging_map_range_identity(PageTable pml4,
                              uint64_t start_address,
                              uint64_t size,
                              uint64_t flags);

int paging_map_range(PageTable pml4,
                     uint64_t virtual_start,
                     uint64_t physical_start,
                     uint64_t size,
                     uint64_t flags);

int paging_map_region_identity(PageTable pml4,
                               MemoryRegionInfo region,
                               uint64_t flags);

#endif
