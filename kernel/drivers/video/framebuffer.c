#include "shared/bootinfo.h"

#include "drivers/video/framebuffer.h"

void fb_put_pixel(FramebufferInfo *fb, uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= fb->width || y >= fb->height)
        return;

    uint32_t *fb_base = (uint32_t *)fb->base;
    fb_base[y * fb->pixels_per_scanline + x] = color;
}

void fb_draw_rectangle(FramebufferInfo *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color)
{
    for (uint32_t j = 0; j < height; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            fb_put_pixel(fb, x + i, y + j, color);
        }
    }
}

void fb_fill_screen(FramebufferInfo *fb, uint32_t color)
{
    for (uint32_t y = 0; y < fb->height; y++)
    {
        for (uint32_t x = 0; x < fb->width; x++)
        {
            fb_put_pixel(fb, x, y, color);
        }
    }
}
