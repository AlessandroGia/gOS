#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

#include "shared/bootinfo.h"

void fb_put_pixel(FramebufferInfo *fb, uint32_t x, uint32_t y, uint32_t color);
void fb_draw_rectangle(FramebufferInfo *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void fb_fill_screen(FramebufferInfo *fb, uint32_t color);

#endif
