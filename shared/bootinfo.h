#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    void *base;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
} FramebufferInfo;

typedef struct
{
    void *base;
    uint64_t size;
    uint64_t descriptor_size;
} MemoryMapInfo;

typedef struct
{
    void *base;
    uint64_t size;
} MemoryRegionInfo;

typedef struct
{
    FramebufferInfo framebuffer;
    MemoryMapInfo memory_map;

    MemoryRegionInfo kernel_image_region;
    MemoryRegionInfo boot_info_region;
    MemoryRegionInfo memory_map_region;
    MemoryRegionInfo framebuffer_region;
} BootInfo;

#endif
