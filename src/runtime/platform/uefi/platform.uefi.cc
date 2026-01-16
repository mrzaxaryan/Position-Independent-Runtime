#if defined(PLATFORM_UEFI)

#include "platform.h"
#include "uefi/efi_system_table.h"

// Microsoft compiler expects _fltused when floating-point is used
// This is required for freestanding UEFI applications
// Note: Both _fltused and __fltused may be needed depending on LTO settings
// The "used" attribute prevents LTO from optimizing these symbols away
extern "C" __attribute__((used)) int _fltused = 0;
extern "C" __attribute__((used)) int __fltused = 0;

// Global UEFI pointers - initialized by Initialize()
EFI_SYSTEM_TABLE *gST = NULL;
EFI_BOOT_SERVICES *gBS = NULL;
EFI_HANDLE gImageHandle = NULL;

VOID Initialize(PENVIRONMENT_DATA envData)
{
    // Initialize UEFI global pointers from environment data
    gImageHandle = envData->ImageHandle;
    gST = envData->SystemTable;
    gBS = envData->SystemTable->BootServices;

    // Mark as not requiring relocation (UEFI handles loading)
    envData->BaseAddress = NULL;
    envData->ShouldRelocate = FALSE;
}

NO_RETURN VOID ExitProcess(USIZE code)
{
    if (gBS && gImageHandle)
    {
        gBS->Exit(gImageHandle, (EFI_STATUS)code, 0, NULL);
    }

    // If Exit returns or we don't have boot services, halt
    while (1)
    {
#if defined(ARCHITECTURE_X86_64) || defined(ARCHITECTURE_I386)
        __asm__ volatile("hlt");
#elif defined(ARCHITECTURE_AARCH64)
        __asm__ volatile("hlt #0");
#elif defined(ARCHITECTURE_ARMV7A)
        __asm__ volatile("wfi");
#endif
    }
}

#endif
