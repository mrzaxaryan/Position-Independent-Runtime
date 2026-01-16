#if defined(PLATFORM_LINUX)

#include "console.h"
#include "linux/syscall.h"
#include "string.h"
#include "memory.h"

UINT32 Console::Write(const CHAR *output, USIZE outputLength)
{
    SSIZE r = Syscall::Write(STDOUT_FILENO, output, outputLength);

    // Linux syscalls return negative errno on error
    if (r < 0)
        return 0;

    return (UINT32)r;
}


UINT32 Console::Write(const WCHAR* text, USIZE wcharCount)
{
    if (!text || !wcharCount)
        return 0;

    const USIZE CHUNK_SIZE = 256;
    WCHAR chunk[CHUNK_SIZE + 1];
    CHAR utf8Buffer[1024];
    UINT32 totalWritten = 0;

    while (wcharCount > 0)
    {
        USIZE currentChunk = (wcharCount > CHUNK_SIZE) ? CHUNK_SIZE : wcharCount;

        Memory::Copy(chunk, text, currentChunk * sizeof(WCHAR));
        chunk[currentChunk] = 0;

        USIZE utf8Len = String::WideToUtf8(chunk, utf8Buffer, sizeof(utf8Buffer));
        totalWritten += Write(utf8Buffer, utf8Len);

        text += currentChunk;
        wcharCount -= currentChunk;
    }

    return totalWritten;
}


#endif