#ifndef GC_KERNEL_FORMAT_H
#define GC_KERNEL_FORMAT_H

#include <stdint.h>

#define GC_MAGIC 0x4347 // "GC" in ASCII
#define GC_VERSION 1
#define KERNEL_LOAD_ADDRESS 0x100000

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;

    uint32_t flags;
    uint32_t reserved;

    uint64_t kernel_address;
    uint64_t entry_point_offset;

    uint64_t payload_file_size;
    uint64_t kernel_memory_size;

} HeaderGC;

#endif
