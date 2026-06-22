#include <efi.h>
#include <efilib.h>

#include "framebuffer.h"

#define KERNEL_LOAD_ADDR 0x100000ULL
#define KERNEL_RESERVE_PAGES 64   /* 256 KB — plenty for now */

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
        ST->ConOut->OutputString(ST->ConOut, L"HandleProtocol(LoadedImage) failed\r\n");
        return Status;
    }

    Status = ST->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (VOID **)&FileSystem);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"HandleProtocol(FileSystem) failed\r\n");
        return Status;
    }

    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"OpenVolume failed\r\n");
        return Status;
    }

    Status = Root->Open(Root, &KernelFile, L"kernel.bin", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"Open(kernel.bin) failed\r\n");
        return Status;
    }

    /* Query file size via EFI_FILE_INFO */
    FileInfoSize = sizeof(EFI_FILE_INFO) + 256;
    Status = ST->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (VOID **)&FileInfo);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"AllocatePool(FileInfo) failed\r\n");
        return Status;
    }

    Status = KernelFile->GetInfo(KernelFile, &GenericFileInfo, &FileInfoSize, FileInfo);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"GetInfo failed\r\n");
        return Status;
    }
    KernelSize = FileInfo->FileSize;
    ST->BootServices->FreePool(FileInfo);

    /* Allocate pages at the fixed physical address the kernel was linked for */
    *LoadAddr = KERNEL_LOAD_ADDR;
    Status = ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, KERNEL_RESERVE_PAGES, LoadAddr);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"AllocatePages failed\r\n");
        return Status;
    }

    ST->BootServices->SetMem((VOID *)(UINTN)*LoadAddr, KERNEL_RESERVE_PAGES * 0x1000, 0);

    Status = KernelFile->Read(KernelFile, &KernelSize, (VOID *)(UINTN)*LoadAddr);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"Read(kernel.bin) failed\r\n");
        return Status;
    }

    KernelFile->Close(KernelFile);
    Root->Close(Root);

    return EFI_SUCCESS;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_PHYSICAL_ADDRESS KernelAddr;

    ST = SystemTable;

    ST->ConOut->OutputString(ST->ConOut, L"ShrimpOS bootloader\r\n");

    Status = load_kernel(ImageHandle, &KernelAddr);
    if (EFI_ERROR(Status))
        return Status;

    ST->ConOut->OutputString(ST->ConOut, L"Kernel loaded\r\n");

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
        ST->ConOut->OutputString(ST->ConOut, L"Unsupported pixel format\r\n");
        return EFI_UNSUPPORTED;
    }

    FrameBuffer fb;
    fb.width = FrameBufferWidth;
    fb.height = FrameBufferHeight;
    fb.base = FrameBufferAddr;
    fb.pitch = PixelsPerScanLine * 4; // 4 bytes per pixel (assuming 32-bit color depth)


    /* ---- ExitBootServices (with retry on stale map key) ---- */
    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey, DescriptorSize;
    UINT32 DescriptorVersion;

    /* Step 1: query size */
    Status = ST->BootServices->GetMemoryMap(
        &MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        ST->ConOut->OutputString(ST->ConOut, L"GetMemoryMap size query failed\r\n");
        return Status;
    }

    /* Step 2: allocate — add slack because AllocatePool itself adds a descriptor */
    MemoryMapSize += 2 * DescriptorSize;
    Status = ST->BootServices->AllocatePool(EfiLoaderData, MemoryMapSize, (VOID **)&MemoryMap);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"AllocatePool(MemoryMap) failed\r\n");
        return Status;
    }

    /* Step 3: fetch real map + valid MapKey */
    Status = ST->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        ST->ConOut->OutputString(ST->ConOut, L"GetMemoryMap failed\r\n");
        return Status;
    }

    /* Step 4: exit — retry once if the key went stale between steps 3 and 4 */
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

    /* ---- Boot services are gone; jump to kernel ---- */
    KernelEntry kernel = (KernelEntry)(UINTN)KernelAddr;
    kernel(&fb);
}
