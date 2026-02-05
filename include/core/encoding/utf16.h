#pragma once

#include "primitives.h"

class UTF16
{
public:
    // Convert a single UTF-16 code unit (or surrogate pair) to UTF-8
    // Returns the number of UTF-8 bytes written (1-4), or 0 if more input needed for surrogate
    // inputIndex is advanced past the consumed code unit(s)
    static USIZE CodepointToUTF8(const WCHAR* input, USIZE inputLength, USIZE& inputIndex, CHAR* output)
    {
        if (inputIndex >= inputLength)
            return 0;

        UINT32 codepoint = input[inputIndex++];

        // Handle UTF-16 surrogate pairs
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF && inputIndex < inputLength)
        {
            UINT32 low = input[inputIndex];
            if (low >= 0xDC00 && low <= 0xDFFF)
            {
                codepoint = 0x10000 + ((codepoint & 0x3FF) << 10) + (low & 0x3FF);
                inputIndex++;
            }
        }

        return CodepointToUTF8Bytes(codepoint, output);
    }

    // Convert a Unicode codepoint to UTF-8 bytes
    // Returns the number of bytes written (1-4)
    static USIZE CodepointToUTF8Bytes(UINT32 codepoint, CHAR* output)
    {
        if (codepoint < 0x80)
        {
            // 1-byte sequence (ASCII)
            output[0] = (CHAR)codepoint;
            return 1;
        }
        else if (codepoint < 0x800)
        {
            // 2-byte sequence
            output[0] = (CHAR)(0xC0 | (codepoint >> 6));
            output[1] = (CHAR)(0x80 | (codepoint & 0x3F));
            return 2;
        }
        else if (codepoint < 0x10000)
        {
            // 3-byte sequence
            output[0] = (CHAR)(0xE0 | (codepoint >> 12));
            output[1] = (CHAR)(0x80 | ((codepoint >> 6) & 0x3F));
            output[2] = (CHAR)(0x80 | (codepoint & 0x3F));
            return 3;
        }
        else if (codepoint < 0x110000)
        {
            // 4-byte sequence
            output[0] = (CHAR)(0xF0 | (codepoint >> 18));
            output[1] = (CHAR)(0x80 | ((codepoint >> 12) & 0x3F));
            output[2] = (CHAR)(0x80 | ((codepoint >> 6) & 0x3F));
            output[3] = (CHAR)(0x80 | (codepoint & 0x3F));
            return 4;
        }

        // Invalid codepoint
        return 0;
    }

    // Convert UTF-16 string to UTF-8, writing to output buffer
    // Returns total number of UTF-8 bytes written
    // outputSize should be at least inputLength * 4 to guarantee no truncation
    static USIZE ToUTF8(const WCHAR* input, USIZE inputLength, CHAR* output, USIZE outputSize)
    {
        USIZE inputIndex = 0;
        USIZE outputIndex = 0;

        while (inputIndex < inputLength && outputIndex < outputSize - 4)
        {
            CHAR utf8Bytes[4];
            USIZE bytesWritten = CodepointToUTF8(input, inputLength, inputIndex, utf8Bytes);

            for (USIZE i = 0; i < bytesWritten && outputIndex < outputSize; i++)
            {
                output[outputIndex++] = utf8Bytes[i];
            }
        }

        return outputIndex;
    }
};
