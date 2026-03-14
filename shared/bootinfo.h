#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    void *framebuffer_base;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pixels_per_scanline;

    void *memory_map;
    uint64_t memory_map_size;
    uint64_t memory_map_descriptor_size;
} BootInfo;

#endif
