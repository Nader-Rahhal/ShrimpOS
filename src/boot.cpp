#include <efi.h>
#include <efilib.h>

#include "framebuffer.h"

#define KERNEL_LOAD_ADDR 0x100000ULL
#define KERNEL_RESERVE_PAGES 64

typedef void __attribute__((sysv_abi)) (*KernelEntry)(FrameBuffer *fb);

static EFI_STATUS load_kernel(EFI_HANDLE ImageHandle, EFI_PHYSICAL_ADDRESS *LoadAddr) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_FILE_IO_INTERFACE *FileSystem;
    EFI_FILE_HANDLE Root, KernelFile;
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize;
    UINTN KernelSize;

    Status = ST->BootServices->HandleProtocol(ImageHandle, &LoadedImageProtocol, (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"HandleProtocol(LoadedImage) failed\r\n");
        return Status;
    }

    Status = ST->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (VOID **)&FileSystem);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"HandleProtocol(FileSystem) failed\r\n");
        return Status;
    }

    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"OpenVolume failed\r\n");
        return Status;
    }

    Status = Root->Open(Root, &KernelFile, (CHAR16*)L"kernel.bin", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"Open(kernel.bin) failed\r\n");
        return Status;
    }

    /* Query file size via EFI_FILE_INFO */
    FileInfoSize = sizeof(EFI_FILE_INFO) + 256;
    Status = ST->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (VOID **)&FileInfo);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"AllocatePool(FileInfo) failed\r\n");
        return Status;
    }

    Status = KernelFile->GetInfo(KernelFile, &GenericFileInfo, &FileInfoSize, FileInfo);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"GetInfo failed\r\n");
        return Status;
    }
    KernelSize = FileInfo->FileSize;
    ST->BootServices->FreePool(FileInfo);

    *LoadAddr = KERNEL_LOAD_ADDR;
    Status = ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, KERNEL_RESERVE_PAGES, LoadAddr);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"AllocatePages failed\r\n");
        return Status;
    }

    ST->BootServices->SetMem((VOID *)(UINTN)*LoadAddr, KERNEL_RESERVE_PAGES * 0x1000, 0);

    Status = KernelFile->Read(KernelFile, &KernelSize, (VOID *)(UINTN)*LoadAddr);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"Read(kernel.bin) failed\r\n");
        return Status;
    }

    KernelFile->Close(KernelFile);
    Root->Close(Root);

    return EFI_SUCCESS;
}

extern "C" EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_PHYSICAL_ADDRESS KernelAddr;

    ST = SystemTable;

    ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"ShrimpOS bootloader\r\n");

    Status = load_kernel(ImageHandle, &KernelAddr);
    if (EFI_ERROR(Status))
        return Status;

    ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"Kernel loaded\r\n");

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    Status = ST->BootServices->LocateProtocol(&GraphicsOutputProtocol, NULL, (VOID **)&gop);
    
    // start of pixel array in memory
    EFI_PHYSICAL_ADDRESS FrameBufferAddr = gop->Mode->FrameBufferBase;

    // size of this array
    UINTN FrameBufferSize = gop->Mode->FrameBufferSize;

    // width and height of the framebuffer (in pixels)
    UINT32 FrameBufferWidth = gop->Mode->Info->HorizontalResolution;
    UINT32 FrameBufferHeight = gop->Mode->Info->VerticalResolution;
    UINT32 PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

    EFI_GRAPHICS_PIXEL_FORMAT PixelFormat = gop->Mode->Info->PixelFormat;

    if (PixelFormat != PixelRedGreenBlueReserved8BitPerColor && PixelFormat != PixelBlueGreenRedReserved8BitPerColor) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"Unsupported pixel format\r\n");
        return EFI_UNSUPPORTED;
    }

    FrameBuffer fb(FrameBufferWidth, FrameBufferHeight, FrameBufferAddr, PixelsPerScanLine * 4);

    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey, DescriptorSize;
    UINT32 DescriptorVersion;

    Status = ST->BootServices->GetMemoryMap(
        &MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"GetMemoryMap size query failed\r\n");
        return Status;
    }

    MemoryMapSize += 2 * DescriptorSize;
    Status = ST->BootServices->AllocatePool(EfiLoaderData, MemoryMapSize, (VOID **)&MemoryMap);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"AllocatePool(MemoryMap) failed\r\n");
        return Status;
    }

    Status = ST->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16*)L"GetMemoryMap failed\r\n");
        return Status;
    }

    UINTN numEntries = MemoryMapSize / DescriptorSize;
    UINT64 totalPages = 0;

    for (UINTN i = 0; i < numEntries; i++) {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemoryMap + i * DescriptorSize);
        if (desc->Type == EfiMemoryMappedIO || desc->Type == EfiMemoryMappedIOPortSpace || desc->Type == EfiUnusableMemory || desc->Type == EfiPalCode || desc->Type == EfiReservedMemoryType || desc->Type == EfiUnacceptedMemoryType) {
            continue;
        }
        totalPages += desc->NumberOfPages;
    }

    Print((CHAR16*)L"Total pages: %llu pages (%llu MB)\r\n", totalPages, (totalPages * 4096) / (1024 * 1024));

    Status = ST->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    Status = ST->BootServices->ExitBootServices(ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        Status = ST->BootServices->GetMemoryMap(
            &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        if (EFI_ERROR(Status))
            return Status;
        Status = ST->BootServices->ExitBootServices(ImageHandle, MapKey);
        if (EFI_ERROR(Status))
            return Status;
    }

    KernelEntry kernel = (KernelEntry)(UINTN)KernelAddr;
    kernel(&fb);

    return EFI_SUCCESS;
}
