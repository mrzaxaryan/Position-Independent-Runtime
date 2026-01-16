#if defined(PLATFORM_WINDOWS)
#pragma once

#include "windows_types.h"
#include "platform.h"
#include "djb2.h"

// NTDLL API Wrappers
class NTDLL
{
private:
public:
	static PVOID RtlAllocateHeap(PVOID HeapHandle, INT32 Flags, USIZE Size);
	static BOOL RtlFreeHeap(PVOID HeapHandle, INT32 Flags, PVOID Pointer);
	static NTSTATUS ZwTerminateProcess(PVOID ProcessHandle, NTSTATUS ExitStatus);
	static PVOID NtCurrentProcess() { return (PVOID)(USIZE)-1L; }
	static PVOID NtCurrentThread() { return (PVOID)(USIZE)-2L; }
};

#endif // PLATFORM_WINDOWS