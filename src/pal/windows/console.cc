#include "console.h"
#include "pal.h"
#include "ntdll.h"
#include "peb.h"

UINT32 Console::Write(const CHAR *text, USIZE length)
{
	PPEB peb = GetCurrentPEB();
	IO_STATUS_BLOCK ioStatusBlock{};
	// Use NtWriteFile - works with both console and redirected output (pipes)
	NTDLL::NtWriteFile(peb->ProcessParameters->StandardOutput, NULL, NULL, NULL, &ioStatusBlock, (PVOID)text, length, NULL, NULL);
	return (UINT32)ioStatusBlock.Information;
}

UINT32 Console::Write(const WCHAR *text, USIZE length)
{
	PPEB peb = GetCurrentPEB();
	IO_STATUS_BLOCK ioStatusBlock{};
	// Use NtWriteFile - works with both console and redirected output (pipes)
	// Note: Writing wide chars directly; length is in characters, multiply by sizeof(WCHAR) for bytes
	NTDLL::NtWriteFile(peb->ProcessParameters->StandardOutput, NULL, NULL, NULL, &ioStatusBlock, (PVOID)text, length * sizeof(WCHAR), NULL, NULL);
	return (UINT32)(ioStatusBlock.Information / sizeof(WCHAR));
}