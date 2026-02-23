#include "console.h"
#include "platform.h"
#include "ntdll.h"
#include "peb.h"
#include "encoding/utf16.h"

UINT32 Console::Write(const CHAR *text, USIZE length)
{
	PPEB peb = GetCurrentPEB();
	IO_STATUS_BLOCK ioStatusBlock;
	Memory::Zero(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));
	NTDLL::ZwWriteFile(peb->ProcessParameters->StandardOutput, NULL, NULL, NULL, &ioStatusBlock, (PVOID)text, length, NULL, NULL);
	return (UINT32)ioStatusBlock.Information;
}

// Write wide string to console (convert UTF-16 to UTF-8 first)
// This ensures ANSI escape codes work properly in terminals and piped output
UINT32 Console::Write(const WCHAR *text, USIZE length)
{
	PPEB peb = GetCurrentPEB();

	constexpr USIZE BUFFER_SIZE = 1024;
	CHAR utf8Buffer[BUFFER_SIZE];
	UINT32 totalWritten = 0;

	USIZE inputIndex = 0;
	while (inputIndex < length)
	{
		USIZE utf8Pos = 0;

		// Convert as many characters as fit in buffer
		while (inputIndex < length && utf8Pos < BUFFER_SIZE - 4)
		{
			CHAR utf8Bytes[4];
			USIZE bytesWritten = UTF16::CodepointToUTF8(text, length, inputIndex, utf8Bytes);

			for (USIZE i = 0; i < bytesWritten; i++)
			{
				utf8Buffer[utf8Pos++] = utf8Bytes[i];
			}
		}

		// Write the converted UTF-8 buffer
		if (utf8Pos > 0)
		{
			IO_STATUS_BLOCK ioStatusBlock;
			Memory::Zero(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));
			NTDLL::ZwWriteFile(peb->ProcessParameters->StandardOutput, NULL, NULL, NULL, &ioStatusBlock, (PVOID)utf8Buffer, utf8Pos, NULL, NULL);
			if (ioStatusBlock.Information > 0)
				totalWritten += (UINT32)ioStatusBlock.Information;
		}
	}

	return totalWritten;
}
