#include "console.h"
#include "platform.h"
#include "ntdll.h"
#include "peb.h"

UINT32 Console::Write(const CHAR *text, USIZE length)
{
	PPEB peb = GetCurrentPEB();
	IO_STATUS_BLOCK ioStatusBlock;
	Memory::Zero(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));
	NTDLL::ZwWriteFile(peb->ProcessParameters->StandardOutput, NULL, NULL, NULL, &ioStatusBlock, (PVOID)text, length, NULL, NULL);
	return (UINT32)ioStatusBlock.Information;
}
