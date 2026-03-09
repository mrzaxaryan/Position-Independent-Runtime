#include "platform/system/machine_id.h"
#include "platform/fs/file.h"
#include "platform/console/logger.h"
#include "core/string/string.h"

Result<UUID, Error> GetMachineUUID()
{
	// /etc/machine-id contains a 32-character hex string (128-bit ID)
	// generated at install time by systemd. Constant across reboots.
	auto openResult = File::Open((const WCHAR *)L"/etc/machine-id"_embed, File::ModeRead);
	if (!openResult)
	{
		LOG_ERROR("Failed to open /etc/machine-id");
		return Result<UUID, Error>::Err(Error(Error::None));
	}

	File &file = openResult.Value();

	// Read the 32 hex characters (may have a trailing newline).
	UINT8 buf[33]{};
	auto readResult = file.Read(Span<UINT8>(buf, 32));
	if (!readResult || readResult.Value() < 32)
	{
		LOG_ERROR("Failed to read /etc/machine-id");
		return Result<UUID, Error>::Err(Error(Error::None));
	}

	// Parse 32 hex characters into 16 UUID bytes.
	// Format as 8-4-4-4-12 for UUID::FromString by inserting dashes.
	CHAR uuidStr[37]{};
	INT32 src = 0;
	INT32 dst = 0;
	for (INT32 i = 0; i < 32; i++)
	{
		if (i == 8 || i == 12 || i == 16 || i == 20)
			uuidStr[dst++] = '-';
		uuidStr[dst++] = (CHAR)buf[src++];
	}
	uuidStr[dst] = '\0';

	return UUID::FromString(Span<const CHAR>(uuidStr, 36));
}
