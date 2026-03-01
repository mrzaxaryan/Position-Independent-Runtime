#pragma once

#include "core/types/primitives.h"
#include "core/types/span.h"
#include "core/types/error.h"
#include "core/types/result.h"
#include "platform/io/file_system/directory_entry.h"

// Class to iterate over directory entries
class DirectoryIterator
{
private:
	PVOID handle;                // Handle to the directory or drive bitmask
	DirectoryEntry currentEntry; // Current directory entry
	BOOL first;                  // Flag for first call to Next()
#ifdef PLATFORM_WINDOWS
	BOOL isBitMaskMode = false; // Flag for bitmask mode on Windows
#endif

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS) || defined(PLATFORM_SOLARIS)
	// Linux/macOS/Solaris-specific (directory entry buffering for getdents64/getdirentries64)
	CHAR buffer[1024];
	INT32 nread;
	INT32 bpos;
#endif

	// Private constructor for factory use only
	DirectoryIterator();

public:
	~DirectoryIterator();

	// Non-copyable
	DirectoryIterator(const DirectoryIterator &) = delete;
	DirectoryIterator &operator=(const DirectoryIterator &) = delete;

	// Movable (transfer ownership of directory handle)
	DirectoryIterator(DirectoryIterator &&other) noexcept;
	DirectoryIterator &operator=(DirectoryIterator &&other) noexcept;

	// Stack-only (placement new allowed for Result<>)
	VOID *operator new(USIZE) = delete;
	VOID operator delete(VOID *) = delete;
	VOID *operator new(USIZE, PVOID ptr) noexcept { return ptr; }

	// Factory — creates and initializes an iterator for the given path
	[[nodiscard]] static Result<DirectoryIterator, Error> Create(PCWCHAR path);

	// Move to next entry. Ok = has entry, Err = done or syscall failed.
	[[nodiscard]] Result<void, Error> Next();
	// Get the current directory entry
	const DirectoryEntry &Get() const { return currentEntry; }
	// Check if the iterator is valid
	BOOL IsValid() const;
};
