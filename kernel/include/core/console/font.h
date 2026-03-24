#ifndef KERNEL_CORE_CONSOLE_FONT_H
#define KERNEL_CORE_CONSOLE_FONT_H

#include <stdint.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define NUM_CHARACTERS 128

extern const uint8_t font8x8_basic[NUM_CHARACTERS][FONT_HEIGHT];

#endif
