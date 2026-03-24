#include "core/panic.h"

#include "core/console/console.h"

void panic_halt(void)
{
    for (;;)
    {
        __asm__ __volatile__("cli");
        __asm__ __volatile__("hlt");
    }
}

void panic(const char *message)
{
    console_set_colors(0x00FFFFFF, 0x00AA0000);
    console_clear();

    console_write("KERNEL PANIC\n\n");

    if (message != 0)
    {
        console_write(message);
        console_putc('\n');
    }

    panic_halt();
}
