#ifndef KERNEL_CORE_EARLY_ALLOCATOR_H
#define KERNEL_CORE_EARLY_ALLOCATOR_H

#include <stdint.h>

#include "core/memory/memory.h"

void early_allocator_init(const MemoryInfo *memory_info,
                          const ReservedRegionsInfo *reserved_info);
void *early_alloc(uint64_t size, uint64_t alignment);
uint64_t early_allocator_remaining(void);

#endif
