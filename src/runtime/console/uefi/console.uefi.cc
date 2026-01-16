#if defined(PLATFORM_UEFI)

#include "console.h"
#include "uefi/efi_system_table.h"
#include "string.h"
#include "memory.h"

UINT32 Console::Write(const CHAR *output, USIZE outputLength)
{
    if (!output || !outputLength || !gST || !gST->ConOut)
        return 0;

    // Convert ASCII to UCS-2 for UEFI console
    // Process in chunks to avoid large stack allocations
    const USIZE CHUNK_SIZE = 256;
    WCHAR buffer[CHUNK_SIZE + 1];
    UINT32 totalWritten = 0;

    while (outputLength > 0)
    {
        USIZE currentChunk = (outputLength > CHUNK_SIZE) ? CHUNK_SIZE : outputLength;

        for (USIZE i = 0; i < currentChunk; i++)
        {
            buffer[i] = (WCHAR)(UINT8)output[i];
        }
        buffer[currentChunk] = 0;

        EFI_STATUS status = gST->ConOut->OutputString(gST->ConOut, buffer);
        if (EFI_ERROR(status))
            break;

        totalWritten += (UINT32)currentChunk;
        output += currentChunk;
        outputLength -= currentChunk;
    }

    return totalWritten;
}

UINT32 Console::Write(const WCHAR *text, USIZE wcharCount)
{
    if (!text || !wcharCount || !gST || !gST->ConOut)
        return 0;

    // UEFI uses UCS-2, same as our WCHAR
    // Process in chunks with null-termination
    const USIZE CHUNK_SIZE = 256;
    WCHAR buffer[CHUNK_SIZE + 1];
    UINT32 totalWritten = 0;

    while (wcharCount > 0)
    {
        USIZE currentChunk = (wcharCount > CHUNK_SIZE) ? CHUNK_SIZE : wcharCount;

        Memory::Copy(buffer, text, currentChunk * sizeof(WCHAR));
        buffer[currentChunk] = 0;

        EFI_STATUS status = gST->ConOut->OutputString(gST->ConOut, buffer);
        if (EFI_ERROR(status))
            break;

        totalWritten += (UINT32)currentChunk;
        text += currentChunk;
        wcharCount -= currentChunk;
    }

    return totalWritten;
}

#endif
