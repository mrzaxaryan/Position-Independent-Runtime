#include "console.h"
#include "encoding/utf16.h"

// Shared UTF-16 to UTF-8 conversion for platforms that output narrow strings.
// UEFI natively uses UTF-16 via EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL, so it provides its own.
#if !defined(PLATFORM_UEFI)

UINT32 Console::Write(const WCHAR *text, USIZE length)
{
	UINT32 totalWritten = 0;

	USIZE inputIndex = 0;
	while (inputIndex < length)
	{
		CHAR utf8Bytes[4];
		USIZE bytesWritten = UTF16::CodepointToUTF8(text, length, inputIndex, utf8Bytes);

		if (bytesWritten > 0)
			totalWritten += Write(utf8Bytes, bytesWritten);
	}

	return totalWritten;
}

#endif
