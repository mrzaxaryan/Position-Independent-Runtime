#pragma once

#include "primitives.h"

class NumberUtils
{
public:
    // Parse a hexadecimal string to UINT32
    // Stops at first non-hex character
    static UINT32 ParseHex(PCCHAR str)
    {
        UINT32 result = 0;
        while (*str != '\0')
        {
            CHAR c = *str;
            UINT32 digit = 0;

            if (c >= '0' && c <= '9')
            {
                digit = c - '0';
            }
            else if (c >= 'a' && c <= 'f')
            {
                digit = 10 + (c - 'a');
            }
            else if (c >= 'A' && c <= 'F')
            {
                digit = 10 + (c - 'A');
            }
            else
            {
                break;
            }

            result = (result << 4) | digit;
            str++;
        }
        return result;
    }

    // Write a decimal number to a buffer
    // Returns pointer to the null terminator
    static PCHAR WriteDecimal(PCHAR buffer, UINT32 num)
    {
        if (num == 0)
        {
            buffer[0] = '0';
            buffer[1] = '\0';
            return buffer + 1;
        }

        CHAR temp[12];
        INT32 i = 0;

        while (num > 0)
        {
            temp[i++] = '0' + (num % 10);
            num /= 10;
        }

        INT32 j = 0;
        while (i > 0)
        {
            buffer[j++] = temp[--i];
        }
        buffer[j] = '\0';
        return buffer + j;
    }

    // Write a hexadecimal number to a buffer (lowercase)
    // Returns pointer to the null terminator
    static PCHAR WriteHex(PCHAR buffer, UINT32 num)
    {
        if (num == 0)
        {
            buffer[0] = '0';
            buffer[1] = '\0';
            return buffer + 1;
        }

        CHAR temp[9];
        INT32 i = 0;

        while (num > 0)
        {
            UINT32 digit = num & 0xF;
            temp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            num >>= 4;
        }

        INT32 j = 0;
        while (i > 0)
        {
            buffer[j++] = temp[--i];
        }
        buffer[j] = '\0';
        return buffer + j;
    }

    // Write a hexadecimal number to a buffer (uppercase)
    // Returns pointer to the null terminator
    static PCHAR WriteHexUpper(PCHAR buffer, UINT32 num)
    {
        if (num == 0)
        {
            buffer[0] = '0';
            buffer[1] = '\0';
            return buffer + 1;
        }

        CHAR temp[9];
        INT32 i = 0;

        while (num > 0)
        {
            UINT32 digit = num & 0xF;
            temp[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
            num >>= 4;
        }

        INT32 j = 0;
        while (i > 0)
        {
            buffer[j++] = temp[--i];
        }
        buffer[j] = '\0';
        return buffer + j;
    }
};
