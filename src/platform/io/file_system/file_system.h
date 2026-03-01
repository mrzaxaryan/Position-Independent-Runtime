#pragma once

#include "core/types/primitives.h"
#include "core/types/error.h"
#include "core/types/result.h"
#include "platform/io/file_system/file.h"

// Class to handle file system operations
class FileSystem
{
public:
	static constexpr INT32 FS_READ = 0x0001;
	static constexpr INT32 FS_WRITE = 0x0002;
	static constexpr INT32 FS_APPEND = 0x0004;
	static constexpr INT32 FS_CREATE = 0x0008;
	static constexpr INT32 FS_TRUNCATE = 0x0010;
	static constexpr INT32 FS_BINARY = 0x0020;

	// File operations
	[[nodiscard]] static Result<File, Error> Open(PCWCHAR path, INT32 flags = 0);
	[[nodiscard]] static Result<void, Error> Delete(PCWCHAR path);
	[[nodiscard]] static Result<void, Error> Exists(PCWCHAR path);

	// New Directory Methods
	[[nodiscard]] static Result<void, Error> CreateDirectory(PCWCHAR path);
	[[nodiscard]] static Result<void, Error> DeleteDirectory(PCWCHAR path);
};
