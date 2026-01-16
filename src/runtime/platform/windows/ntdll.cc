#include "ntdll.h"

#define ResolveNtdllExportAddress(functionName) ResolveExportAddressFromPebModule(Djb2::HashCompileTime(L"ntdll.dll"), Djb2::HashCompileTime(functionName))


PVOID NTDLL::RtlAllocateHeap(PVOID HeapHandle, INT32 Flags, USIZE Size)
{
    return ((PVOID(STDCALL *)(PVOID HeapHandle, INT32 Flags, USIZE Size))ResolveNtdllExportAddress("RtlAllocateHeap"))(HeapHandle, Flags, Size);
}

BOOL NTDLL::RtlFreeHeap(PVOID HeapHandle, INT32 Flags, PVOID Pointer)
{
    return ((BOOL(STDCALL *)(PVOID HeapHandle, INT32 Flags, PVOID Pointer))ResolveNtdllExportAddress("RtlFreeHeap"))(HeapHandle, Flags, Pointer);
}

NTSTATUS NTDLL::ZwTerminateProcess(PVOID ProcessHandle, NTSTATUS ExitStatus)
{
    return ((NTSTATUS(STDCALL *)(PVOID ProcessHandle, NTSTATUS ExitStatus))ResolveNtdllExportAddress("ZwTerminateProcess"))(ProcessHandle, ExitStatus);
}
