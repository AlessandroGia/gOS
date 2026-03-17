#include <efi/efi.h>
#include <efi/efilib.h>

#include "uefi/apps/loader/kernel/bin/kernel.h"

#include "uefi/common/helper/helper.h"
#include "uefi/common/log/log.h"

static EFI_STATUS allocate_kernel_pages(
    EFI_SYSTEM_TABLE *SystemTable,
    EFI_PHYSICAL_ADDRESS *destination,
    UINTN kernel_size)
{

    EFI_STATUS Status;
    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePages, 4,
        AllocateAddress,
        EfiLoaderData,
        EFI_SIZE_TO_PAGES(kernel_size),
        destination);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to allocate pages for kernel: %r", Status);
        return Status;
    }

    return EFI_SUCCESS;
}

static void copy_kernel_to_address(
    EFI_PHYSICAL_ADDRESS *destination,
    UINTN kernel_size,
    VOID *kernel_buffer)
{

    CHAR8 *src = (CHAR8 *)kernel_buffer;
    CHAR8 *dst = (CHAR8 *)(*destination);

    for (UINTN i = 0; i < kernel_size; i++)
    {
        dst[i] = src[i];
    }
}

EFI_STATUS load_kernel_to_address(
    EFI_SYSTEM_TABLE *SystemTable,
    EFI_PHYSICAL_ADDRESS *destination,
    UINTN kernel_size,
    VOID *kernel_buffer)
{
    EFI_STATUS Status;

    Status = allocate_kernel_pages(SystemTable, destination, kernel_size);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR(L"Failed to allocate pages for kernel: %r", Status);
        return Status;
    }
    copy_kernel_to_address(destination, kernel_size, kernel_buffer);
    return EFI_SUCCESS;
}

EFI_STATUS load_kernel_file(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *st,
    VOID **kernel_buffer,
    UINTN *kernel_size)
{
    EFI_STATUS Status;

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_PROTOCOL *Root = NULL;
    EFI_FILE_PROTOCOL *KernelFile = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    UINTN FileInfoSize = 0;

    *kernel_buffer = NULL;
    *kernel_size = 0;

    Status = uefi_call_wrapper(
        st->BootServices->OpenProtocol, 6,
        ImageHandle,
        &LoadedImageProtocol,
        (VOID **)&LoadedImage,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to open LoadedImage protocol: %r", Status);
        goto cleanup;
    } // LoadedImage->DeviceHandle should now point to the device containing the kernel file

    Status = uefi_call_wrapper(
        st->BootServices->OpenProtocol, 6,
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to open SimpleFileSystem protocol: %r", Status);
        goto cleanup;
    } // FileSystem should now point to the filesystem containing the kernel file

    Status = uefi_call_wrapper(
        FileSystem->OpenVolume, 2,
        FileSystem,
        &Root);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to open root volume: %r", Status);
        goto cleanup;
    } // Root should now point to the root directory of the filesystem

    Status = uefi_call_wrapper(
        Root->Open, 5,
        Root,
        &KernelFile,
        L"\\kernel.bin",
        EFI_FILE_MODE_READ,
        0);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to open kernel.bin: %r", Status);
        goto cleanup;
    } // KernelFile should now point to the kernel file

    FileInfoSize = 0;
    Status = uefi_call_wrapper(
        KernelFile->GetInfo, 4,
        KernelFile,
        &gEfiFileInfoGuid,
        &FileInfoSize,
        NULL);
    if (Status != EFI_BUFFER_TOO_SMALL)
    {
        LOG_ERROR("Failed to query kernel file info size: %r", Status);
        goto cleanup;
    } // FileInfoSize should now contain the size of the buffer needed to hold the file info

    Status = uefi_call_wrapper(
        st->BootServices->AllocatePool, 3,
        EfiLoaderData,
        FileInfoSize,
        (VOID **)&FileInfo);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to allocate file info buffer: %r", Status);
        goto cleanup;
    } // FileInfo should now point to a buffer large enough to hold the file info

    Status = uefi_call_wrapper(
        KernelFile->GetInfo,
        4,
        KernelFile,
        &gEfiFileInfoGuid,
        &FileInfoSize,
        FileInfo);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to get kernel file info: %r", Status);
        goto cleanup;
    } // FileInfo should now contain the file info for the kernel file, including its size

    *kernel_size = (UINTN)FileInfo->FileSize;

    Status = uefi_call_wrapper(
        st->BootServices->AllocatePool,
        3,
        EfiLoaderData,
        *kernel_size,
        kernel_buffer);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to allocate kernel buffer: %r", Status);
        goto cleanup;
    } // kernel_buffer should now point to a buffer large enough to hold the kernel file

    Status = uefi_call_wrapper(
        KernelFile->Read,
        3,
        KernelFile,
        kernel_size,
        *kernel_buffer);
    if (EFI_ERROR(Status))
    {
        LOG_ERROR("Failed to read kernel file: %r", Status);
        goto cleanup;
    } // At this point, kernel_buffer should contain the contents of the kernel file and kernel_size should contain its size

cleanup:
    if (FileInfo)
        free_pool(st, FileInfo);
    if (KernelFile)
        uefi_call_wrapper(KernelFile->Close, 1, KernelFile);
    if (Root)
        uefi_call_wrapper(Root->Close, 1, Root);
    if (EFI_ERROR(Status) && *kernel_buffer)
    {
        free_pool(st, *kernel_buffer);
        *kernel_buffer = NULL;
        *kernel_size = 0;
    }

    return Status;
}
