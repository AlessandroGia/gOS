#include <stdint.h>

#include "shared/uefi_memory.h"

#include "core/memory_map.h"
#include "core/console/console.h"
#include "lib/kstring.h"

static const char *memory_type_to_string(uint32_t type)
{
    switch (type)
    {
    case 0:
        return "Reserved";
    case 1:
        return "LoaderCode";
    case 2:
        return "LoaderData";
    case 3:
        return "BootServicesCode";
    case 4:
        return "BootServicesData";
    case 5:
        return "RuntimeServicesCode";
    case 6:
        return "RuntimeServicesData";
    case 7:
        return "ConventionalMemory";
    case 8:
        return "UnusableMemory";
    case 9:
        return "ACPIReclaimMemory";
    case 10:
        return "ACPIMemoryNVS";
    case 11:
        return "MemoryMappedIO";
    case 12:
        return "MemoryMappedIOPortSpace";
    case 13:
        return "PalCode";
    case 14:
        return "PersistentMemory";
    default:
        return "Unknown";
    }
}

static uint64_t get_memory_size_by_desc(const UefiMemoryDescriptor *desc)
{
    return desc->number_of_pages * 4096; // Assuming 4KB pages
}

uint64_t get_memory_size_by_type(const MemoryMapInfo *memory_map, uint32_t type)
{
    if (memory_map == 0 || memory_map->base == 0 || memory_map->descriptor_size == 0)
    {
        return 0;
    }

    uint64_t descriptor_count = memory_map->size / memory_map->descriptor_size;
    uint64_t total_size = 0;

    for (uint64_t i = 0; i < descriptor_count; i++)
    {
        uint8_t *entry_ptr = (uint8_t *)memory_map->base + (i * memory_map->descriptor_size);
        const UefiMemoryDescriptor *desc = (UefiMemoryDescriptor *)entry_ptr;

        if (desc->type == type)
        {
            total_size += get_memory_size_by_desc(desc);
        }
    }

    return total_size;
}

void memory_map_dump(const MemoryMapInfo *memory_map)
{
    if (memory_map == 0 || memory_map->base == 0 || memory_map->descriptor_size == 0)
    {
        console_write("Memory map not available.\n");
        return;
    }

    uint64_t descriptor_count = memory_map->size / memory_map->descriptor_size;

    console_write("Memory map:\n");
    console_write("  descriptors: ");
    console_write_dec_u64(descriptor_count);
    console_putc('\n');

    console_write("  descriptor_size: ");
    console_write_dec_u64(memory_map->descriptor_size);
    console_putc('\n');
    console_putc('\n');

    for (uint64_t i = 0; i < descriptor_count; i++)
    {
        uint8_t *entry_ptr = (uint8_t *)memory_map->base + (i * memory_map->descriptor_size);
        UefiMemoryDescriptor *desc = (UefiMemoryDescriptor *)entry_ptr;

        console_write("[");
        console_write_dec_u64(i);
        console_write("] ");

        console_write(memory_type_to_string(desc->type));
        console_putc('\n');

        console_write("  type: ");
        console_write_dec_u32(desc->type);
        console_putc('\n');

        console_write("  physical_start: 0x");
        console_write_hex_u64((uint64_t)desc->physical_start);
        console_putc('\n');

        console_write("  number_of_pages: ");
        console_write_dec_u64(desc->number_of_pages);
        console_putc('\n');

        console_write("  attribute: 0x");
        console_write_hex_u64(desc->attribute);
        console_putc('\n');

        console_putc('\n');
    }
}
