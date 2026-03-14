#include <stdint.h>

#include "shared/bootinfo.h"

void kernel_main(BootInfo *boot_info)
{
    uint32_t *fb = (uint32_t *)boot_info->framebuffer_base;

    for (uint32_t y = 0; y < boot_info->framebuffer_height; y++)
    {
        for (uint32_t x = 0; x < boot_info->framebuffer_width; x++)
        {
            fb[y * boot_info->framebuffer_pixels_per_scanline + x] = 0x0000FF00;
        }
    }

    for (;;)
    {
        __asm__ __volatile__("hlt");
    }
}
