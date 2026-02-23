#include "allocator.h"
#include "ntdll.h"
#include "windows_types.h"

PVOID Allocator::AllocateMemory(USIZE len)
{
    PVOID base = NULL;
    USIZE size = len;
    NTSTATUS status = NTDLL::ZwAllocateVirtualMemory(NTDLL::NtCurrentProcess(), &base, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return NT_SUCCESS(status) ? base : NULL;
}

VOID Allocator::ReleaseMemory(PVOID ptr, USIZE)
{
    USIZE size = 0;
    NTDLL::ZwFreeVirtualMemory(NTDLL::NtCurrentProcess(), &ptr, &size, MEM_RELEASE);
}