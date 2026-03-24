#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include "shared/bootinfo.h"

void memory_map_dump(const MemoryMapInfo *memory_map);
uint64_t get_memory_size_by_type(const MemoryMapInfo *memory_map, uint32_t type);

#endif
