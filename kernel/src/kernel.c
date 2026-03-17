/*
#include <stdint.h>

#include "shared/bootinfo.h"

void kernel_main(BootInfo *boot_info)
{
    uint32_t *fb = (uint32_t *)boot_info->framebuffer_base;

    for (uint32_t y = 0; y < boot_info->framebuffer_height; y++)
    {
        for (uint32_t x = 0; x < boot_info->framebuffer_width; x++)
        {
            fb[y * boot_info->framebuffer_pixels_per_scanline + x] = x * 0x10000 + y * 0x100 + 0xFF;
        }
    }

    for (;;)
    {
        __asm__ __volatile__("hlt");
    }
}
*/
#include <stdint.h>

#include "shared/bootinfo.h"

void put_pixel(BootInfo *boot_info, uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= boot_info->framebuffer_width || y >= boot_info->framebuffer_height)
        return;

    uint32_t *fb = (uint32_t *)boot_info->framebuffer_base;
    fb[y * boot_info->framebuffer_pixels_per_scanline + x] = color;
}

void draw_rectangle(BootInfo *boot_info, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color)
{
    for (uint32_t j = 0; j < height; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            put_pixel(boot_info, x + i, y + j, color);
        }
    }
}

void fill_screen(BootInfo *boot_info, uint32_t color)
{
    for (uint32_t y = 0; y < boot_info->framebuffer_height; y++)
    {
        for (uint32_t x = 0; x < boot_info->framebuffer_width; x++)
        {
            put_pixel(boot_info, x, y, color);
        }
    }
}

void draw_char(BootInfo *boot_info, char c, uint32_t x, uint32_t y, uint32_t color)
{
    // Simple 8x8 font for demonstration (only supports characters 32-127)
    static const uint8_t font[96][8] = {
        // ... (font data would go here)
    };

    if (c < 32 || c > 127)
        return;

    const uint8_t *glyph = font[c - 32];
    for (uint32_t j = 0; j < 8; j++)
    {
        for (uint32_t i = 0; i < 8; i++)
        {
            if (glyph[j] & (1 << (7 - i)))
            {
                put_pixel(boot_info, x + i, y + j, color);
            }
        }
    }
}

void kernel_main(BootInfo *boot_info)
{
    fill_screen(boot_info, 0x00FF00);
    draw_rectangle(boot_info, 0, 0, 200, 200, 0xFF0000);
    for (;;)
    {
        __asm__ __volatile__("hlt");
    }
}
