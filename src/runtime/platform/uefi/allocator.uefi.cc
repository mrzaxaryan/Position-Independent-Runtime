#if defined(PLATFORM_UEFI)

#include "allocator.h"
#include "uefi/efi_system_table.h"

PVOID Allocator::AllocateMemory(USIZE size)
{
    PVOID buffer = NULL;

    EFI_STATUS status = gBS->AllocatePool(
        EfiLoaderData,
        size,
        &buffer);

    if (EFI_ERROR(status))
        return NULL;

    return buffer;
}

VOID Allocator::ReleaseMemory(PVOID ptr, USIZE sizeHint)
{
    (VOID)sizeHint;

    if (!ptr)
        return;

    gBS->FreePool(ptr);
}

#endif
