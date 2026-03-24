# GOS

UEFI bootloader & kernel POC

## Description

# 1. Bootloader Responsibilities

The UEFI bootloader is responsible for preparing the execution environment for the kernel. Its first task is to open the kernel file and parse the custom kernel header (`HeaderGC`). This header is the contract between the build pipeline, the bootloader, and the kernel, and provides the information needed to load the kernel correctly into memory.

The bootloader extracts at least the following fields from the header:

* `kernel_address`: physical load address of the kernel image
* `payload_file_size`: number of initialized bytes actually present in the file payload
* `kernel_memory_size`: total runtime memory size required by the kernel image
* `entry_point_offset`: entry point relative to the kernel base address

The distinction between `payload_file_size` and `kernel_memory_size` is essential. The former describes what is physically stored in the kernel file, while the latter describes the total amount of memory that the kernel must occupy at runtime, including memory that is not explicitly stored in the file, such as `.bss`.

---

# 2. Kernel Image Loading

Once the bootloader has parsed the header, it allocates enough physical pages to contain the full runtime image of the kernel. This step is performed through the UEFI boot services, using a routine such as `allocate_kernel_pages(...)`.

After successful allocation, the kernel image is loaded in two phases:

1. **Payload copy**: the first `payload_file_size` bytes are copied from the file buffer into the destination physical address using `copy_kernel_to_address(...)`.
2. **Runtime zero-fill**: the remaining bytes, from `payload_file_size` up to `kernel_memory_size`, are initialized to zero using `zero_memory_at_address(...)`.

This second phase is required because sections such as `.bss` are not stored in the file as actual bytes, but must exist in memory at runtime and must start in a zero-initialized state.

The full loading sequence is coordinated by `load_kernel_to_address(...)`.

At the end of this phase, the kernel exists as a valid runtime image in physical memory, not just as a file payload.

---

# 3. Linker Script and Entry Code

The in-memory structure of the kernel is not decided by the bootloader, but by the kernel linker script (`kernel.ld`). The linker script defines:

* base address of the image
* ordering of sections
* alignment constraints
* layout of `.text`, `.rodata`, `.data`, and `.bss`

The earliest entry code is provided by `entry.S`. This file defines:

* the low-level entry symbol used by the kernel
* the primitive initial stack
* the transition from early assembly code into the C-level kernel entry function

In the current design, the kernel is loaded at physical address `0x100000`, and execution starts from the entry point derived from:

```text
kernel_address + entry_point_offset
```

Because the stack is defined in `.bss`, the distinction between file payload size and runtime memory size is critical: the stack exists at runtime even though its bytes are not explicitly stored in the kernel file.

---

# 4. Boot Information Contract (`BootInfo`)

At the end of the bootloader phase, control is transferred to the kernel together with a `BootInfo` structure. This structure is the initial data contract between the loader and the kernel.

`BootInfo` contains at least the following logical groups of information:

* **Framebuffer information**: base address, dimensions, pixels per scanline
* **UEFI memory map information**: raw memory map buffer, total size, descriptor size
* **Kernel image region**: physical base and runtime size of the loaded kernel image
* **BootInfo region**: physical location and size of the `BootInfo` structure itself
* **Memory map region**: physical location and size of the memory map buffer
* **Framebuffer region**: physical location and size of the framebuffer memory area

The purpose of these region descriptors is to make explicit which physical regions are already in use at the moment control reaches the kernel.

---

# 5. Console Subsystem

The kernel implements a minimal framebuffer console as one of its earliest subsystems. This console writes directly to the framebuffer and provides enough functionality to support early boot debugging.

The console subsystem includes functions such as:

* `console_init(...)`
* `console_clear()`
* `console_putc(...)`
* `console_write(...)`
* `console_write_hex_u64(...)`
* `console_write_dec_u64(...)`

Text rendering is performed using a simple 8x8 bitmap font. At this stage the console is not a terminal abstraction; it is a direct framebuffer writer designed to provide visibility during the earliest kernel phases.

---

# 6. Memory Map Analysis

Once inside the kernel, one of the first steps is to analyze the UEFI memory map passed by the bootloader.

This is handled by `memory_init(...)`, which scans the descriptors contained in the raw memory map buffer and builds a higher-level summary structure, typically `MemoryInfo`.

This summary contains at least:

* total physical memory
* total usable physical memory
* base of the largest usable region
* size of the largest usable region

The kernel can later retrieve this information using `memory_get_info()`.

At this level, memory-related helper functions such as `memory_region_overlaps(...)` are also used to reason about whether two memory regions overlap. These helpers belong to the general memory domain, not to allocation or paging specifically.

---

# 7. Early Allocator

The kernel requires dynamic memory before a full heap or page-frame allocator exists. To satisfy this need, an early linear allocator is used.

This allocator is initialized with `early_allocator_init(...)`, which receives:

* information about usable physical memory
* a list of reserved regions to avoid (`ReservedRegionsInfo`)

The allocator uses a bump-pointer strategy over an early usable region:

* `g_early_base`
* `g_early_current`
* `g_early_end`

Allocations are performed with `early_alloc(size, alignment)`, which returns aligned memory from the early region and advances the internal cursor.

The allocator also exposes inspection helpers such as:

* `early_allocator_remaining()`
* `early_allocator_base()`
* `early_allocator_current()`

This allocator is intentionally simple. It is not a full heap and does not support deallocation. Its purpose is to support the bootstrap phase, especially page table creation and other early kernel structures.

---

# 8. Alignment as a Fundamental Constraint

A major concept introduced during this phase is alignment.

Many low-level structures require specific alignment properties. In particular, x86_64 page tables must:

* occupy exactly 4096 bytes
* be aligned on 4096-byte boundaries

To support this, the code makes use of alignment helpers such as:

* `align_up(...)`
* `align_down(...)`

These functions do not move memory. They only transform addresses mathematically so that arbitrary regions can be expanded to page boundaries or allocations can satisfy architectural constraints.

Alignment is used both in the early allocator and later in the paging subsystem when whole regions must be mapped page-by-page.

---

# 9. Paging Structures

The kernel implements a first x86_64 paging subsystem based on the standard four-level hierarchy:

* `PML4`
* `PDPT`
* `PD`
* `PT`

Each table:

* occupies one physical page (4096 bytes)
* contains 512 entries
* uses entries of 8 bytes each

A page table is therefore both a logical paging structure and a concrete physical page allocated in RAM.

Page tables are allocated using `paging_alloc_table()`, which internally uses the early allocator to obtain a 4096-byte, 4096-aligned block of memory and zeroes it before use.

---

# 10. Page Table Entry Helpers

The paging subsystem provides a set of low-level helpers to manipulate and interpret page table entries:

* `paging_set_entry(...)`
* `paging_get_entry_address(...)`
* `paging_entry_present(...)`
* `paging_get_indices(...)`

`paging_get_indices(...)` decomposes a virtual address into its component indices:

* PML4 index
* PDPT index
* PD index
* PT index

These indices are then used to walk or construct the paging hierarchy.

`paging_entry_present(...)` checks whether an entry has the `Present` bit set. This determines whether a level of the hierarchy already exists or needs to be allocated.

---

# 11. Single-Page Mapping Primitive

The core paging primitive is `paging_map_page(...)`.

This function takes:

* a root `PML4`
* a virtual address
* a physical address
* a set of flags

and constructs the mapping for a single 4 KiB page.

Its logic is as follows:

1. verify that the virtual and physical addresses are page-aligned
2. extract paging indices from the virtual address
3. walk the paging hierarchy from PML4 down to PT
4. allocate missing intermediate tables (`PDPT`, `PD`, `PT`) if they do not yet exist
5. finally install the physical frame mapping in the terminal PT entry

This function does not allocate the final physical frame being mapped. It only allocates intermediate page tables when needed. The final physical address is assumed to already refer to an existing physical page.

This distinction is crucial: **mapping is not allocation**.

---

# 12. Range and Region Mapping

On top of the single-page primitive, the kernel implements higher-level mapping helpers:

* `paging_map_range(...)`
* `paging_map_range_identity(...)`
* `paging_map_region_identity(...)`

`paging_map_range(...)` maps a range of pages by repeatedly calling `paging_map_page(...)` while advancing both virtual and physical addresses page-by-page.

`paging_map_range_identity(...)` is a specialization where:

```text
virtual address = physical address
```

`paging_map_region_identity(...)` takes a `MemoryRegionInfo`, aligns its start and end to page boundaries, and identity-maps all pages covering that region.

This mechanism is used to map entire boot-critical regions without requiring each region to already be page-aligned.

---

# 13. Identity Mapping During Bootstrap

In the bootstrap phase, the chosen strategy is to use a first **identity mapping**.

This means that the kernel initially builds a virtual address space where:

```text
VA == PA
```

for the regions it considers critical.

Identity mapping is not the final design goal of a mature kernel, but it greatly simplifies the first paging transition because:

* code continues to execute at the same apparent addresses
* the stack remains reachable using the same numeric values
* existing pointers remain valid under the new translation model

This avoids the need for an immediate jump to higher-half addresses or a more advanced virtual memory layout.

---

# 14. Regions Mapped During Bootstrap

The first address space built by the kernel includes at least the following regions:

* the kernel image itself (`kernel_image`)
* the `BootInfo` structure (`boot_info_region`)
* the raw memory map buffer (`memory_map_region`)
* the framebuffer (`framebuffer_region`)

These are mapped because they are all required for the kernel to continue executing immediately after the paging transition.

In addition to these, the kernel also maps a wider **early allocator window**. This is not because every byte in that window is already allocated, but because the window contains:

* page tables already allocated
* page tables likely to be allocated immediately afterwards
* potentially other early allocations

This solves the bootstrap problem where page tables themselves must remain accessible after the address space switch.

---

# 15. Page Tables as Memory to Be Kept Reachable

A subtle but critical point is that page tables are not abstract entities outside memory: they are ordinary physical pages used with a special purpose.

This means they must also be reachable after the paging switch.

The bootstrap solution adopted here is pragmatic:

* allocate page tables from the early allocator
* identity-map a sufficiently large early memory window
* ensure that both already allocated and near-future page tables remain accessible inside that window

This does not mean the whole window is reserved exclusively for page tables. It means that the kernel guarantees accessibility of that part of physical memory while bootstrap is still ongoing.

---

# 16. Loading the New Address Space (`CR3` Switch)

Once the initial paging hierarchy is constructed and the critical regions are mapped, the kernel performs the transition to its own address space by loading the physical address of the new `PML4` into register `CR3`.

This operation is encapsulated in `paging_load_pml4(...)`.

Conceptually, this step tells the CPU:

> from now on, use this page table root to translate virtual addresses

At this point, the CPU stops using the previous paging root and starts using the one created by the kernel.

Because the kernel uses identity mapping for this first transition, the execution flow can continue without an immediate change in the apparent addresses used by the code.

---

# 17. What Was Achieved

At the end of this work, the kernel has reached a major milestone.

It now:

* loads a runtime-correct image from the bootloader
* receives a richer and more accurate `BootInfo`
* understands the physical memory layout at a basic level
* can allocate memory in early boot through a linear allocator
* can allocate and initialize x86_64 page tables
* can construct its own paging hierarchy
* can map critical physical regions into a virtual address space
* can replace the active page table root by loading its own `PML4` into `CR3`
* continues executing correctly after the switch

This means the kernel is no longer merely loaded into RAM; it is already able to define and use its own initial virtual memory environment.

---

# 18. Current Architectural State

The current system is still a bootstrap-stage kernel, not yet a fully mature memory manager. In particular, it does not yet implement:

* a full physical page-frame allocator
* a complete virtual memory manager
* a general-purpose kernel heap
* fine-grained page permissions per section
* unmapping and reclamation policies
* advanced layouts such as a higher-half kernel

However, the foundations required for all of those are now in place.

The kernel has crossed the critical threshold from:

> being merely loaded and executed by the bootloader

into:

> being able to manage the first version of its own address space.


## Requirements

- Make
- Docker
- Docker Compose

- QEMU instelled on host system
- OVMF firmware file

## Setup

Place OVFM firware (OVMF_CODE.fd) in assets folder.
Builds are executed inside Docker, while QEMU is run on the host system.

## Usage

### Build the Docker image

Run this the first time, or whenever the `Dockerfile` changes:

```bash
make image
```

### Build the project

```bash
make build
```

This generates the UEFI binary in the build directory.

### Run the VM

```bash
make run
```

This prepares the UEFI boot path and starts QEMU with OVMF.

### Clean generated files

```bash
make clean
```

## License

This project is licensed under the MIT License.

See [LICENSE](LICENSE) for details.
