#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/gc_kernel_format.h"

#define EI_NIDENT 16
#define PT_LOAD 1

typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

static int read_exact(FILE *file, void *buffer, size_t size)
{
    return fread(buffer, 1, size, file) == size ? 0 : -1;
}

static int write_all(FILE *file, const void *buffer, size_t size)
{
    return fwrite(buffer, 1, size, file) == size ? 0 : -1;
}

static int is_valid_elf64(const Elf64_Ehdr *ehdr)
{
    return ehdr->e_ident[0] == 0x7F &&
           ehdr->e_ident[1] == 'E' &&
           ehdr->e_ident[2] == 'L' &&
           ehdr->e_ident[3] == 'F' &&
           ehdr->e_ident[4] == 2 &&
           ehdr->e_ident[5] == 1;
}

int main(int argc, char **argv)
{
    const char *input_path;
    const char *output_path;
    FILE *input = NULL;
    FILE *output = NULL;

    Elf64_Ehdr ehdr;
    Elf64_Phdr *phdrs = NULL;
    uint8_t *payload = NULL;

    uint64_t load_base = UINT64_MAX;
    uint64_t payload_file_end = 0;
    uint64_t memory_end = 0;
    uint64_t entry_offset = 0;

    HeaderGC header;
    size_t phdr_table_size;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <kernel.elf> <kernel.GC>\n", argv[0]);
        return 1;
    }

    input_path = argv[1];
    output_path = argv[2];

    input = fopen(input_path, "rb");
    if (input == NULL)
    {
        fprintf(stderr, "Error opening input '%s': %s\n", input_path, strerror(errno));
        return 1;
    }

    if (read_exact(input, &ehdr, sizeof(ehdr)) != 0)
    {
        fprintf(stderr, "Error reading ELF header from '%s'\n", input_path);
        fclose(input);
        return 1;
    }

    if (!is_valid_elf64(&ehdr))
    {
        fprintf(stderr, "Input '%s' is not a valid ELF64 little-endian file\n", input_path);
        fclose(input);
        return 1;
    }

    if (ehdr.e_phentsize != sizeof(Elf64_Phdr))
    {
        fprintf(stderr, "Unexpected program header size in '%s'\n", input_path);
        fclose(input);
        return 1;
    }

    phdr_table_size = (size_t)ehdr.e_phnum * sizeof(Elf64_Phdr);
    phdrs = (Elf64_Phdr *)malloc(phdr_table_size);
    if (phdrs == NULL)
    {
        fprintf(stderr, "Out of memory allocating program headers\n");
        fclose(input);
        return 1;
    }

    if (fseek(input, (long)ehdr.e_phoff, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error seeking to program headers in '%s'\n", input_path);
        free(phdrs);
        fclose(input);
        return 1;
    }

    if (read_exact(input, phdrs, phdr_table_size) != 0)
    {
        fprintf(stderr, "Error reading program headers from '%s'\n", input_path);
        free(phdrs);
        fclose(input);
        return 1;
    }

    for (uint16_t i = 0; i < ehdr.e_phnum; i++)
    {
        const Elf64_Phdr *ph = &phdrs[i];

        if (ph->p_type != PT_LOAD)
        {
            continue;
        }

        if (ph->p_memsz == 0)
        {
            continue;
        }

        if (ph->p_paddr < load_base)
        {
            load_base = ph->p_paddr;
        }

        if (ph->p_paddr + ph->p_filesz > payload_file_end)
        {
            payload_file_end = ph->p_paddr + ph->p_filesz;
        }

        if (ph->p_paddr + ph->p_memsz > memory_end)
        {
            memory_end = ph->p_paddr + ph->p_memsz;
        }
    }

    if (load_base == UINT64_MAX || memory_end <= load_base || payload_file_end < load_base)
    {
        fprintf(stderr, "No valid PT_LOAD segments found in '%s'\n", input_path);
        free(phdrs);
        fclose(input);
        return 1;
    }

    uint64_t payload_file_size = payload_file_end - load_base;
    uint64_t kernel_memory_size = memory_end - load_base;

    if (ehdr.e_entry < load_base || ehdr.e_entry >= memory_end)
    {
        fprintf(stderr, "Entry point is outside loaded image range\n");
        free(phdrs);
        fclose(input);
        return 1;
    }

    entry_offset = ehdr.e_entry - load_base;

    payload = (uint8_t *)calloc(1, (size_t)payload_file_size);
    if (payload == NULL)
    {
        fprintf(stderr, "Out of memory allocating payload buffer\n");
        free(phdrs);
        fclose(input);
        return 1;
    }

    for (uint16_t i = 0; i < ehdr.e_phnum; i++)
    {
        const Elf64_Phdr *ph = &phdrs[i];

        if (ph->p_type != PT_LOAD || ph->p_filesz == 0)
        {
            continue;
        }

        uint64_t dest_offset = ph->p_paddr - load_base;

        if (fseek(input, (long)ph->p_offset, SEEK_SET) != 0)
        {
            fprintf(stderr, "Error seeking to segment data in '%s'\n", input_path);
            free(payload);
            free(phdrs);
            fclose(input);
            return 1;
        }

        if (read_exact(input, payload + dest_offset, (size_t)ph->p_filesz) != 0)
        {
            fprintf(stderr, "Error reading segment data from '%s'\n", input_path);
            free(payload);
            free(phdrs);
            fclose(input);
            return 1;
        }
    }

    fclose(input);
    input = NULL;
    free(phdrs);
    phdrs = NULL;

    memset(&header, 0, sizeof(header));
    header.magic = GC_MAGIC;
    header.version = GC_VERSION;
    header.header_size = (uint16_t)sizeof(header);
    header.flags = 0;
    header.kernel_address = load_base;
    header.entry_point_offset = entry_offset;
    header.payload_file_size = payload_file_size;
    header.kernel_memory_size = kernel_memory_size;

    output = fopen(output_path, "wb");
    if (output == NULL)
    {
        fprintf(stderr, "Error opening output '%s': %s\n", output_path, strerror(errno));
        free(payload);
        return 1;
    }

    if (write_all(output, &header, sizeof(header)) != 0)
    {
        fprintf(stderr, "Error writing header to '%s'\n", output_path);
        free(payload);
        fclose(output);
        return 1;
    }

    if (write_all(output, payload, (size_t)payload_file_size) != 0)
    {
        fprintf(stderr, "Error writing payload to '%s'\n", output_path);
        free(payload);
        fclose(output);
        return 1;
    }

    free(payload);
    fclose(output);

    printf("Created %s\n", output_path);
    printf("  magic            : 0x%08X\n", header.magic);
    printf("  version          : %u\n", header.version);
    printf("  header_size      : %u\n", header.header_size);
    printf("  load_addr        : 0x%016llX\n", (unsigned long long)header.kernel_address);
    printf("  entry_offset     : 0x%016llX\n", (unsigned long long)header.entry_point_offset);
    printf("  payload_file_size: %llu bytes\n", (unsigned long long)header.payload_file_size);
    printf("  kernel_mem_size  : %llu bytes\n", (unsigned long long)header.kernel_memory_size);

    return 0;
}
