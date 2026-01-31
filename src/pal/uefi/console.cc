/**
 * console.cc - UEFI Console I/O Implementation
 *
 * Provides Console::Write using EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.
 * UEFI natively uses CHAR16 (UTF-16), which maps to our WCHAR.
 * Includes ANSI escape sequence parsing for color support.
 */

#include "console.h"
#include "efi_context.h"
#include "memory.h"

// =============================================================================
// ANSI Escape Sequence Parser
// =============================================================================

/**
 * AnsiToEfiColor - Convert ANSI SGR code to EFI text attribute
 *
 * Handles common ANSI SGR (Select Graphic Rendition) codes:
 * - 0     = Reset to default (light gray on black)
 * - 30-37 = Set foreground color
 *
 * @param code - ANSI SGR code number
 * @return EFI text attribute
 */
static USIZE AnsiToEfiColor(INT32 code)
{
	switch (code)
	{
	case 0:
		return EFI_LIGHTGRAY; // Reset to default
	case 30:
		return EFI_BLACK;
	case 31:
		return EFI_LIGHTRED; // Red
	case 32:
		return EFI_LIGHTGREEN; // Green
	case 33:
		return EFI_YELLOW; // Yellow
	case 34:
		return EFI_LIGHTBLUE; // Blue
	case 35:
		return EFI_LIGHTMAGENTA; // Magenta
	case 36:
		return EFI_LIGHTCYAN; // Cyan
	case 37:
		return EFI_WHITE; // White
	default:
		return EFI_LIGHTGRAY;
	}
}

/**
 * OutputWithAnsiParsing - Output wide string with ANSI escape sequence parsing
 *
 * Parses ANSI escape sequences of the form:
 *   ESC [ <code> ; <code> m
 *   ESC [ <code> m
 *
 * @param conOut - EFI console output protocol
 * @param text   - Text to output
 * @param length - Length of text
 * @return Number of visible characters written
 */
static UINT32 OutputWithAnsiParsingW(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *conOut, const WCHAR *text, USIZE length)
{
	constexpr USIZE BUFFER_SIZE = 256;
	WCHAR buffer[BUFFER_SIZE];
	USIZE bufIdx = 0;
	UINT32 totalWritten = 0;

	for (USIZE i = 0; i < length; i++)
	{
		// Check for ESC character (0x1B or 033)
		if (text[i] == L'\033' && i + 1 < length && text[i + 1] == L'[')
		{
			// Flush any pending text before processing escape sequence
			if (bufIdx > 0)
			{
				buffer[bufIdx] = L'\0';
				conOut->OutputString(conOut, buffer);
				totalWritten += bufIdx;
				bufIdx = 0;
			}

			// Parse escape sequence: ESC [ <codes> m
			i += 2; // Skip ESC and [

			// Parse numeric codes (e.g., "0;32" or "0" or "31")
			INT32 codes[4] = {0, 0, 0, 0};
			INT32 codeCount = 0;
			INT32 currentCode = 0;
			BOOL hasCode = FALSE;

			while (i < length && text[i] != L'm')
			{
				if (text[i] >= L'0' && text[i] <= L'9')
				{
					currentCode = currentCode * 10 + (text[i] - L'0');
					hasCode = TRUE;
				}
				else if (text[i] == L';')
				{
					if (hasCode && codeCount < 4)
					{
						codes[codeCount++] = currentCode;
					}
					currentCode = 0;
					hasCode = FALSE;
				}
				i++;
			}

			// Store final code
			if (hasCode && codeCount < 4)
			{
				codes[codeCount++] = currentCode;
			}

			// Apply the color attribute
			// We look for foreground color codes (30-37) or reset (0)
			USIZE attr = EFI_LIGHTGRAY;
			for (INT32 j = 0; j < codeCount; j++)
			{
				if (codes[j] == 0)
				{
					attr = EFI_LIGHTGRAY; // Reset
				}
				else if (codes[j] >= 30 && codes[j] <= 37)
				{
					attr = AnsiToEfiColor(codes[j]);
				}
			}

			conOut->SetAttribute(conOut, attr);
			// Note: 'i' now points to 'm', loop will increment to next character
		}
		else
		{
			// Regular character - add to buffer
			buffer[bufIdx++] = text[i];

			// Flush if buffer is full
			if (bufIdx >= BUFFER_SIZE - 1)
			{
				buffer[bufIdx] = L'\0';
				conOut->OutputString(conOut, buffer);
				totalWritten += bufIdx;
				bufIdx = 0;
			}
		}
	}

	// Flush remaining text
	if (bufIdx > 0)
	{
		buffer[bufIdx] = L'\0';
		conOut->OutputString(conOut, buffer);
		totalWritten += bufIdx;
	}

	return totalWritten;
}

/**
 * OutputWithAnsiParsing (CHAR version) - Output narrow string with ANSI parsing
 */
static UINT32 OutputWithAnsiParsingC(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *conOut, const CHAR *text, USIZE length)
{
	constexpr USIZE BUFFER_SIZE = 256;
	WCHAR buffer[BUFFER_SIZE];
	USIZE bufIdx = 0;
	UINT32 totalWritten = 0;

	for (USIZE i = 0; i < length; i++)
	{
		// Check for ESC character (0x1B or 033)
		if (text[i] == '\033' && i + 1 < length && text[i + 1] == '[')
		{
			// Flush any pending text before processing escape sequence
			if (bufIdx > 0)
			{
				buffer[bufIdx] = L'\0';
				conOut->OutputString(conOut, buffer);
				totalWritten += bufIdx;
				bufIdx = 0;
			}

			// Parse escape sequence: ESC [ <codes> m
			i += 2; // Skip ESC and [

			// Parse numeric codes
			INT32 codes[4] = {0, 0, 0, 0};
			INT32 codeCount = 0;
			INT32 currentCode = 0;
			BOOL hasCode = FALSE;

			while (i < length && text[i] != 'm')
			{
				if (text[i] >= '0' && text[i] <= '9')
				{
					currentCode = currentCode * 10 + (text[i] - '0');
					hasCode = TRUE;
				}
				else if (text[i] == ';')
				{
					if (hasCode && codeCount < 4)
					{
						codes[codeCount++] = currentCode;
					}
					currentCode = 0;
					hasCode = FALSE;
				}
				i++;
			}

			// Store final code
			if (hasCode && codeCount < 4)
			{
				codes[codeCount++] = currentCode;
			}

			// Apply the color attribute
			USIZE attr = EFI_LIGHTGRAY;
			for (INT32 j = 0; j < codeCount; j++)
			{
				if (codes[j] == 0)
				{
					attr = EFI_LIGHTGRAY;
				}
				else if (codes[j] >= 30 && codes[j] <= 37)
				{
					attr = AnsiToEfiColor(codes[j]);
				}
			}

			conOut->SetAttribute(conOut, attr);
		}
		else
		{
			// Regular character - convert to wide and add to buffer
			buffer[bufIdx++] = (WCHAR)(UINT8)text[i];

			// Flush if buffer is full
			if (bufIdx >= BUFFER_SIZE - 1)
			{
				buffer[bufIdx] = L'\0';
				conOut->OutputString(conOut, buffer);
				totalWritten += bufIdx;
				bufIdx = 0;
			}
		}
	}

	// Flush remaining text
	if (bufIdx > 0)
	{
		buffer[bufIdx] = L'\0';
		conOut->OutputString(conOut, buffer);
		totalWritten += bufIdx;
	}

	return totalWritten;
}

// =============================================================================
// Console::Write Implementations
// =============================================================================

/**
 * Console::Write (WCHAR) - Output wide string to UEFI console
 *
 * UEFI natively uses CHAR16 (identical to our WCHAR with -fshort-wchar),
 * so this is the primary output path. Parses ANSI escape sequences for colors.
 *
 * @param text   - Pointer to wide character string
 * @param length - Number of characters to write
 * @return Number of characters written
 */
UINT32 Console::Write(const WCHAR *text, USIZE length)
{
	if (text == NULL || length == 0)
		return 0;

	EFI_CONTEXT *ctx = GetEfiContext();
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *conOut = ctx->SystemTable->ConOut;

	if (conOut == NULL)
		return 0;

	return OutputWithAnsiParsingW(conOut, text, length);
}

/**
 * Console::Write (CHAR) - Output narrow string to UEFI console
 *
 * Converts ASCII/Latin-1 to CHAR16 (UTF-16) for UEFI output.
 * Parses ANSI escape sequences for colors.
 *
 * @param text   - Pointer to narrow character string
 * @param length - Number of characters to write
 * @return Number of characters written
 */
UINT32 Console::Write(const CHAR *text, USIZE length)
{
	if (text == NULL || length == 0)
		return 0;

	EFI_CONTEXT *ctx = GetEfiContext();
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *conOut = ctx->SystemTable->ConOut;

	if (conOut == NULL)
		return 0;

	return OutputWithAnsiParsingC(conOut, text, length);
}
