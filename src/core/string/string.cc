#include "string.h"

// Converts a UTF-8 string to wide string (UTF-16)
// Returns the number of wide characters written (excluding null terminator)
USIZE String::Utf8ToWide(PCCHAR utf8, PWCHAR wide, USIZE wideBufferSize)
{
    if (!utf8 || !wide || wideBufferSize == 0)
        return 0;

    USIZE wideLen = 0;

    while (*utf8 && wideLen < wideBufferSize - 2)
    {
        UINT32 ch;
        UINT8 byte = (UINT8)*utf8++;

        if (byte < 0x80)
        {
            ch = byte;
        }
        else if ((byte & 0xE0) == 0xC0)
        {
            ch = (byte & 0x1F) << 6;
            if (*utf8)
                ch |= (*utf8++ & 0x3F);
        }
        else if ((byte & 0xF0) == 0xE0)
        {
            ch = (byte & 0x0F) << 12;
            if (*utf8)
                ch |= (*utf8++ & 0x3F) << 6;
            if (*utf8)
                ch |= (*utf8++ & 0x3F);
        }
        else if ((byte & 0xF8) == 0xF0)
        {
            ch = (byte & 0x07) << 18;
            if (*utf8)
                ch |= (*utf8++ & 0x3F) << 12;
            if (*utf8)
                ch |= (*utf8++ & 0x3F) << 6;
            if (*utf8)
                ch |= (*utf8++ & 0x3F);

            // Encode as surrogate pair for characters > 0xFFFF
            if (ch >= 0x10000)
            {
                ch -= 0x10000;
                wide[wideLen++] = (WCHAR)(0xD800 + (ch >> 10));
                wide[wideLen++] = (WCHAR)(0xDC00 + (ch & 0x3FF));
                continue;
            }
        }
        else
        {
            continue; // Invalid UTF-8 byte, skip
        }

        wide[wideLen++] = (WCHAR)ch;
    }

    wide[wideLen] = L'\0';
    return wideLen;
}

