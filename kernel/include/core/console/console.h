#ifndef KERNEL_CORE_CONSOLE_CONSOLE_H
#define KERNEL_CORE_CONSOLE_CONSOLE_H

#include <stdint.h>
#include <shared/bootinfo.h>

void console_init(FramebufferInfo *framebuffer);
void console_set_colors(uint32_t foreground, uint32_t background);
void console_clear(void);
void console_putc(char c);
void console_write(const char *str);

void console_write_hex_u64(uint64_t value);
void console_write_hex_u32(uint32_t value);

void console_write_dec_u32(uint32_t value);
void console_write_dec_u64(uint64_t value);

#endif
