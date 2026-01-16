#pragma once

#include "primitives.h"

class Allocator
{
private:
    /* data */
public:
    static PVOID AllocateMemory(USIZE size);
    static VOID ReleaseMemory(PVOID ptr, USIZE size);

    static PVOID CopyMemory(PVOID dest, PCVOID src, USIZE count);
    static INT32 CompareMemory(PCVOID ptr1, PCVOID ptr2, USIZE num);
    static PVOID SetMemory(PVOID dest, INT32 ch, USIZE count);
};
