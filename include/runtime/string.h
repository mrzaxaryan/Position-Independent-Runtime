#pragma once

#include "primitives.h"
#include "platform.h"

class String
{
private:
public:
    template <TCHAR TChar>
    static USIZE Length(const TChar *pChar);
    template <TCHAR TChar>
    static TChar ToLowerCase(TChar c);

    static USIZE WideToUtf8(PCWCHAR wide, PCHAR utf8, USIZE utf8BufferSize);
};

// Converts a wide string (UTF-16) to UTF-8
// Returns the number of bytes written (excluding null terminator)
inline USIZE String::WideToUtf8(PCWCHAR wide, PCHAR utf8, USIZE utf8BufferSize)
{
    if (!wide || !utf8 || utf8BufferSize == 0)
        return 0;

    USIZE utf8Len = 0;

    while (*wide && utf8Len < utf8BufferSize - 4)
    {
        UINT32 ch = *wide++;

        // Handle surrogate pairs for characters > 0xFFFF
        if (ch >= 0xD800 && ch <= 0xDBFF && *wide >= 0xDC00 && *wide <= 0xDFFF)
        {
            ch = 0x10000 + ((ch - 0xD800) << 10) + (*wide++ - 0xDC00);
        }

        if (ch < 0x80)
        {
            utf8[utf8Len++] = (CHAR)ch;
        }
        else if (ch < 0x800)
        {
            utf8[utf8Len++] = (CHAR)(0xC0 | (ch >> 6));
            utf8[utf8Len++] = (CHAR)(0x80 | (ch & 0x3F));
        }
        else if (ch < 0x10000)
        {
            utf8[utf8Len++] = (CHAR)(0xE0 | (ch >> 12));
            utf8[utf8Len++] = (CHAR)(0x80 | ((ch >> 6) & 0x3F));
            utf8[utf8Len++] = (CHAR)(0x80 | (ch & 0x3F));
        }
        else
        {
            utf8[utf8Len++] = (CHAR)(0xF0 | (ch >> 18));
            utf8[utf8Len++] = (CHAR)(0x80 | ((ch >> 12) & 0x3F));
            utf8[utf8Len++] = (CHAR)(0x80 | ((ch >> 6) & 0x3F));
            utf8[utf8Len++] = (CHAR)(0x80 | (ch & 0x3F));
        }
    }

    utf8[utf8Len] = '\0';
    return utf8Len;
}

/**
 * ToLowerCase - Convert character to lowercase
 *
 * FORCE_INLINE: Called frequently in string formatting and parsing
 * Hot path: Format specifier parsing, hash calculations
 */
template <TCHAR TChar>
FORCE_INLINE TChar String::ToLowerCase(TChar c)
{
	if (c >= (TChar)'A' && c <= (TChar)'Z')
	{
		return c + ((TChar)'a' - (TChar)'A');
	}
	return c;
}

/**
 * Length - Calculate null-terminated string length
 *
 * FORCE_INLINE: Called on every Console::Write() with embedded strings
 * Critical hot path: Avoid function call overhead for this simple loop
 *
 * @param p - Null-terminated string
 * @return Number of characters before null terminator
 */
template <TCHAR TChar>
FORCE_INLINE USIZE String::Length(const TChar *p)
{
	USIZE i = 0;

	// Loop until null terminator
	while (p[i] != (TChar)'\0')
	{
		i++;
	}

	return i;
}
