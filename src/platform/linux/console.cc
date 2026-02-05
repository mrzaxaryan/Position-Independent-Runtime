#include "console.h"
#include "syscall.h"
#include "system.h"
#include "encoding/utf16.h"

// Write ANSI/ASCII string to console (straightforward)
UINT32 Console::Write(const CHAR *text, USIZE length)
{
    SSIZE result = System::Call(SYS_WRITE, STDOUT_FILENO, (USIZE)text, length);
    return (result >= 0) ? (UINT32)result : 0;
}

// Write wide string to console (convert UTF-16 to UTF-8 first)
UINT32 Console::Write(const WCHAR *text, USIZE length)
{
    constexpr USIZE BUFFER_SIZE = 1024;
    CHAR utf8Buffer[BUFFER_SIZE];
    UINT32 totalWritten = 0;

    USIZE inputIndex = 0;
    while (inputIndex < length)
    {
        USIZE utf8Pos = 0;

        // Convert as many characters as fit in buffer
        while (inputIndex < length && utf8Pos < BUFFER_SIZE - 4)
        {
            CHAR utf8Bytes[4];
            USIZE bytesWritten = UTF16::CodepointToUTF8(text, length, inputIndex, utf8Bytes);

            for (USIZE i = 0; i < bytesWritten; i++)
            {
                utf8Buffer[utf8Pos++] = utf8Bytes[i];
            }
        }

        // Write the converted UTF-8 buffer
        if (utf8Pos > 0)
        {
            SSIZE written = System::Call(SYS_WRITE, STDOUT_FILENO, (USIZE)utf8Buffer, utf8Pos);
            if (written > 0)
                totalWritten += written;
        }
    }

    return totalWritten;
}
