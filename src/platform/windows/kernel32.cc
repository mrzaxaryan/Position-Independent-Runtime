#include "kernel32.h"
#include "platform.h"
#include "peb.h"

#define ResolveKernel32ExportAddress(functionName) ResolveExportAddressFromPebModule(Djb2::HashCompileTime(L"kernel32.dll"), Djb2::HashCompileTime(functionName))

// CreateProcessW
BOOL Kernel32::CreateProcessW(PWCHAR lpApplicationName, PWCHAR lpCommandLine, PVOID lpProcessAttributes, PVOID lpThreadAttributes, BOOL bInheritHandles, UINT32 dwCreationFlags, PVOID lpEnvironment, PWCHAR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    return ((BOOL(STDCALL *)(PWCHAR lpApplicationName, PWCHAR lpCommandLine, PVOID lpProcessAttributes, PVOID lpThreadAttributes, BOOL bInheritHandles, UINT32 dwCreationFlags, PVOID lpEnvironment, PWCHAR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation))ResolveKernel32ExportAddress("CreateProcessW"))(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

// SetHandleInformation
BOOL Kernel32::SetHandleInformation(PVOID hObject, UINT32 dwMask, UINT32 dwFlags)
{
    return ((BOOL(STDCALL *)(PVOID hObject, UINT32 dwMask, UINT32 dwFlags))ResolveKernel32ExportAddress("SetHandleInformation"))(hObject, dwMask, dwFlags);
}