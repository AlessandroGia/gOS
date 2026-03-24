#include "core/console/console.h"
#include "core/console/font.h"
#include "drivers/video/framebuffer.h"

static FramebufferInfo *g_framebuffer = 0;
static uint32_t g_cursor_x = 0;
static uint32_t g_cursor_y = 0;
static uint32_t g_foreground = 0x00FFFFFF;
static uint32_t g_background = 0x00000000;

static void console_newline(void)
{
    g_cursor_x = 0;
    g_cursor_y += FONT_HEIGHT;
}

static void console_draw_char(char c, uint32_t x, uint32_t y)
{
    if (g_framebuffer == 0)
    {
        return;
    }

    if ((unsigned char)c >= 128)
    {
        return;
    }

    for (uint32_t row = 0; row < FONT_HEIGHT; row++)
    {
        uint8_t bits = font8x8_basic[(unsigned char)c][row];

        for (uint32_t col = 0; col < FONT_WIDTH; col++)
        {
            uint32_t color = (bits & (1 << (7 - col))) ? g_foreground : g_background;
            fb_put_pixel(g_framebuffer, x + col, y + row, color);
        }
    }
}

void console_init(FramebufferInfo *framebuffer)
{
    g_framebuffer = framebuffer;
    g_cursor_x = 0;
    g_cursor_y = 0;
    g_foreground = 0x00FFFFFF;
    g_background = 0x00000000;
}

void console_set_colors(uint32_t foreground, uint32_t background)
{
    g_foreground = foreground;
    g_background = background;
}

void console_clear(void)
{
    if (g_framebuffer == 0)
    {
        return;
    }

    fb_fill_screen(g_framebuffer, g_background);
    g_cursor_x = 0;
    g_cursor_y = 0;
}

void console_putc(char c)
{
    if (g_framebuffer == 0)
    {
        return;
    }

    if (c == '\n')
    {
        console_newline();
        return;
    }

    if (g_cursor_x + FONT_WIDTH > g_framebuffer->width)
    {
        console_newline();
    }

    if (g_cursor_y + FONT_HEIGHT > g_framebuffer->height)
    {
        return;
    }

    console_draw_char(c, g_cursor_x, g_cursor_y);
    g_cursor_x += FONT_WIDTH;
}

void console_write(const char *str)
{
    if (str == 0)
    {
        return;
    }

    while (*str != '\0')
    {
        console_putc(*str);
        str++;
    }
}

static char hex_digit(uint8_t value)
{
    if (value < 10)
    {
        return (char)('0' + value);
    }

    return (char)('A' + (value - 10));
}

void console_write_hex_u32(uint32_t value)
{
    char buffer[9];
    buffer[8] = '\0';

    for (int shift = 28; shift >= 0; shift -= 4)
    {
        uint8_t nibble = (uint8_t)((value >> shift) & 0xF);
        buffer[7 - shift / 4] = hex_digit(nibble);
    }

    uint8_t index = 0;
    while (index < 7 && buffer[index] == '0')
    {
        index++;
    }

    console_write(buffer + index);
}

void console_write_hex_u64(uint64_t value)
{
    char buffer[17];
    buffer[16] = '\0';

    for (int shift = 60; shift >= 0; shift -= 4)
    {
        uint8_t nibble = (uint8_t)((value >> shift) & 0xF);
        buffer[15 - shift / 4] = hex_digit(nibble);
    }

    uint8_t index = 0;
    while (index < 15 && buffer[index] == '0')
    {
        index++;
    }

    console_write(buffer + index);
}

void console_write_dec_u32(uint32_t value)
{
    char buffer[10];
    int index = 0;

    if (value == 0)
    {
        console_putc('0');
        return;
    }

    while (value > 0)
    {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0)
    {
        console_putc(buffer[--index]);
    }
}

void console_write_dec_u64(uint64_t value)
{
    char buffer[20];
    int index = 0;

    if (value == 0)
    {
        console_putc('0');
        return;
    }

    while (value > 0)
    {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0)
    {
        console_putc(buffer[--index]);
    }
}
