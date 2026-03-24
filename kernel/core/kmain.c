#include <stdint.h>

#include "shared/bootinfo.h"

#include "core/console/console.h"
#include "core/memory_map.h"
#include "core/memory/memory.h"
#include "core/panic.h"
#include "core/early_allocator.h"
#include "arch/x86_64/memory/paging.h"

#include "lib/kmemory.h"

static void kernel_halt(void)
{
    for (;;)
    {
        __asm__ __volatile__("hlt");
    }
}

static void print_region(const char *name, MemoryRegionInfo region)
{
    console_write(name);
    console_write(" base=0x");
    console_write_hex_u64((uint64_t)region.base);
    console_write(" size=");
    console_write_dec_u64(region.size);
    console_putc('\n');
}

static void map_region_or_panic(PageTable pml4,
                                const char *name,
                                MemoryRegionInfo region,
                                uint64_t flags)
{
    console_write("Mapping ");
    console_write(name);
    console_putc('\n');

    if (!paging_map_region_identity(pml4, region, flags))
    {
        panic("failed to map region");
    }
}

void kernel_main(BootInfo *boot_info)
{
    if (boot_info == 0)
    {
        kernel_halt();
    }

    console_init(&boot_info->framebuffer);
    console_set_colors(0x00FFFFFF, 0x00000000);
    console_clear();

    memory_init(&boot_info->memory_map);

    const MemoryInfo *memory_info = memory_get_info();

    MemoryRegionInfo reserved_regions[] =
        {
            boot_info->kernel_image_region,
            boot_info->boot_info_region,
            boot_info->memory_map_region,
            boot_info->framebuffer_region};

    ReservedRegionsInfo reserved_info =
        {
            .regions = reserved_regions,
            .count = sizeof(reserved_regions) / sizeof(reserved_regions[0])};

    early_allocator_init(memory_info, &reserved_info);

    PageTable pml4 = paging_alloc_table();

    if (pml4 == 0)
    {

        panic("failed to allocate pml4");
    }

    console_write("Regions:\n");
    print_region("kernel_image", boot_info->kernel_image_region);
    print_region("boot_info_region", boot_info->boot_info_region);
    print_region("memory_map_region", boot_info->memory_map_region);
    print_region("framebuffer_region", boot_info->framebuffer_region);
    console_putc('\n');

    map_region_or_panic(pml4, "kernel_image", boot_info->kernel_image_region, PAGE_PRESENT | PAGE_WRITABLE);
    map_region_or_panic(pml4, "boot_info_region", boot_info->boot_info_region, PAGE_PRESENT | PAGE_WRITABLE);
    map_region_or_panic(pml4, "memory_map_region", boot_info->memory_map_region, PAGE_PRESENT | PAGE_WRITABLE);
    map_region_or_panic(pml4, "framebuffer_region", boot_info->framebuffer_region, PAGE_PRESENT | PAGE_WRITABLE);

    console_write("Paging regions mapped.\n");

    console_write("pml4: 0x");
    console_write_hex_u64((uint64_t)pml4);
    console_putc('\n');

    panic_halt();

    kernel_halt();
}
