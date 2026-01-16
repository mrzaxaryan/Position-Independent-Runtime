#include "allocator.h"

PVOID operator new(USIZE size)
{
    return Allocator::AllocateMemory(size);
}

PVOID operator new[](USIZE size)
{
    return Allocator::AllocateMemory(size);
}

VOID operator delete(PVOID p) noexcept
{
    Allocator::ReleaseMemory(p, 0);
}

VOID operator delete[](PVOID p) noexcept
{
    Allocator::ReleaseMemory(p, 0);
}

VOID operator delete(PVOID p, USIZE s) noexcept
{
    Allocator::ReleaseMemory(p, s);
}

VOID operator delete[](PVOID p, USIZE s) noexcept
{
    Allocator::ReleaseMemory(p, s);
}

extern "C" PVOID memset(PVOID dest, INT32 ch, USIZE count)
{
    PCHAR p = (PCHAR)dest;
    CHAR byte = (CHAR)ch;

    for (USIZE i = 0; i < count; i++)
    {
        p[i] = byte;
    }

    return dest;
}

extern "C" PVOID memcpy(PVOID dest, const VOID *src, USIZE count)
{
    if (!dest || !src || count == 0)
        return dest;

    PUCHAR d = (PUCHAR)dest;
    const UCHAR *s = (const UCHAR *)src;

    for (USIZE i = 0; i < count; i++)
        d[i] = s[i];

    return dest;
}

extern "C" INT32 memcmp(const VOID *ptr1, const VOID *ptr2, USIZE num)
{
    const UCHAR *p1 = (const UCHAR *)ptr1;
    const UCHAR *p2 = (const UCHAR *)ptr2;

    for (USIZE i = 0; i < num; i++)
    {
        if (p1[i] != p2[i])
            return (INT32)(p1[i] - p2[i]);
    }

    return 0;
}

PVOID Allocator::CopyMemory(PVOID dest, PCVOID src, USIZE count)
{
    return memcpy(dest, src, count);
}

INT32 Allocator::CompareMemory(PCVOID ptr1, PCVOID ptr2, USIZE num)
{
    return memcmp(ptr1, ptr2, num);
}

PVOID Allocator::SetMemory(PVOID dest, INT32 ch, USIZE count)
{
    return memset(dest, ch, count);
}