#ifndef UEFI_MEMORY_H
#define UEFI_MEMORY_H

#include <stdint.h>

typedef struct
{
    uint32_t type;
    void *physical_start;
    void *virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
} UefiMemoryDescriptor;

#endif
