/**
 * @file memory.cc
 * @brief CRT-free memory operation implementations.
 * @details Byte-by-byte implementations of memset, memcpy, and memcmp
 * conforming to ISO/IEC 9899:2018 (C17) semantics.
 */

#include "core/memory/memory.h"
#include "core/compiler/compiler.h"

extern "C" COMPILER_RUNTIME PVOID memset(PVOID dest, INT32 ch, USIZE count)
{
	PUCHAR p = (PUCHAR)dest;
	UCHAR byte = (UCHAR)ch;

	for (USIZE i = 0; i < count; i++)
	{
		p[i] = byte;
	}

	return dest;
}

extern "C" COMPILER_RUNTIME PVOID memcpy(PVOID dest, const VOID *src, USIZE count)
{
	PUCHAR d = (PUCHAR)dest;
	const UCHAR *s = (const UCHAR *)src;

	for (USIZE i = 0; i < count; i++)
		d[i] = s[i];

	return dest;
}

extern "C" COMPILER_RUNTIME INT32 memcmp(const VOID *ptr1, const VOID *ptr2, USIZE num)
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