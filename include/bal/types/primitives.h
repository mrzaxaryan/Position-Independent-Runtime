#pragma once

#include "compiler.h"

#define TRUE true
#define FALSE false
#define NULL nullptr

typedef void VOID, *PVOID, **PPVOID;
typedef const void *PCVOID, **PPCVOID;

typedef signed char INT8, *PINT8;
typedef unsigned char UINT8, *PUINT8, **PPUINT8;

typedef signed short INT16, *PINT16;
typedef unsigned short UINT16, *PUINT16;

typedef signed int INT32, *PINT32;
typedef unsigned int UINT32, *PUINT32, **PPUINT32;

typedef signed long long INT64, *PINT64, **PPINT64;
typedef unsigned long long UINT64, *PUINT64, **PPUINT64;

// Helper macros for accessing high/low 32-bits of 64-bit integers
#define UINT64_LOW(val) ((UINT32)((val) & 0xFFFFFFFFULL))
#define UINT64_HIGH(val) ((UINT32)(((val) >> 32) & 0xFFFFFFFFULL))
#define INT64_LOW(val) ((UINT32)((val) & 0xFFFFFFFFLL))
#define INT64_HIGH(val) ((INT32)(((val) >> 32) & 0xFFFFFFFFLL))

// Helper function to make 64-bit value from high and low parts
#define MAKE_UINT64(high, low) ((((UINT64)(high)) << 32) | ((UINT64)(low)))
#define MAKE_INT64(high, low) ((INT64)(((((UINT64)(UINT32)(high))) << 32) | ((UINT64)(low))))

// Parse a string to INT64 (replacement for INT64::Parse)
static inline INT64 ParseINT64(const char *str) noexcept
{
    INT64 num = 0;
    INT32 sign = 1;

    if (str == nullptr)
        return 0;

    // Skip leading whitespace
    while (*str == ' ')
        str++;

    // Check for optional sign
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    // Convert string to integer
    while (*str >= '0' && *str <= '9')
    {
        num = num * 10 + (*str - '0');
        str++;
    }

    return sign < 0 ? -num : num;
}

typedef char CHAR, *PCHAR, **PPCHAR;
typedef unsigned char UCHAR, *PUCHAR;
typedef const CHAR *PCCHAR;

typedef float FLOAT, *PFLOAT;

typedef wchar_t WCHAR, *PWCHAR, **PPWCHAR;
typedef const WCHAR *PCWCHAR;

typedef bool BOOL, *PBOOL, **PPBOOL;

typedef __SIZE_TYPE__ USIZE, *PUSIZE;
typedef __INTPTR_TYPE__ SSIZE, *PSSIZE;

typedef __builtin_va_list VA_LIST;
#define VA_START(ap, v) __builtin_va_start(ap, v)
#define VA_ARG(ap, t) __builtin_va_arg(ap, t)
#define VA_END(ap) __builtin_va_end(ap)

#if defined(PLATFORM_WINDOWS_I386)
#define STDCALL __attribute__((stdcall))
#elif defined(PLATFORM_WINDOWS_X86_64)
#define STDCALL __attribute__((ms_abi))
#elif defined(PLATFORM_WINDOWS_ARMV7A)
#define STDCALL
#elif defined(PLATFORM_WINDOWS_AARCH64)
#define STDCALL
#elif defined(PLATFORM_LINUX)
#define STDCALL  // Linux uses System V ABI, no special calling convention needed
#else
#define STDCALL  // Default: no special calling convention
#endif

