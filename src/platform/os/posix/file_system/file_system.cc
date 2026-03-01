#include "platform/io/file_system/file_system.h"
#include "core/memory/memory.h"
#if defined(PLATFORM_LINUX)
#include "platform/os/linux/common/syscall.h"
#include "platform/os/linux/common/system.h"
#elif defined(PLATFORM_MACOS)
#include "platform/os/macos/common/syscall.h"
#include "platform/os/macos/common/system.h"
#elif defined(PLATFORM_SOLARIS)
#include "platform/os/solaris/common/syscall.h"
#include "platform/os/solaris/common/system.h"
#endif
#include "core/string/string.h"
#include "core/encoding/utf16.h"
#include "platform/io/file_system/path.h"

// =============================================================================
// Helper: Normalize a wide path to a null-terminated UTF-8 string.
// Centralises the normalise + convert sequence that every FileSystem
// method needs, avoiding 2 KB of stack per inlined copy.
// =============================================================================

static NOINLINE USIZE NormalizePathToUtf8(PCWCHAR path, Span<CHAR> utf8Out)
{
	WCHAR normalizedPath[1024];
	USIZE pathLen = Path::NormalizePath(path, Span<WCHAR>(normalizedPath));
	USIZE utf8Len = UTF16::ToUTF8(Span<const WCHAR>(normalizedPath, pathLen),
								   Span<CHAR>(utf8Out.Data(), utf8Out.Size() - 1));
	utf8Out.Data()[utf8Len] = '\0';
	return utf8Len;
}

// =============================================================================
// FileSystem Implementation
// =============================================================================

Result<File, Error> FileSystem::Open(PCWCHAR path, INT32 flags)
{
	CHAR utf8Path[1024];
	NormalizePathToUtf8(path, Span<CHAR>(utf8Path));

	INT32 openFlags = 0;
	INT32 mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	// Access mode
	if ((flags & FS_READ) && (flags & FS_WRITE))
		openFlags |= O_RDWR;
	else if (flags & FS_WRITE)
		openFlags |= O_WRONLY;
	else
		openFlags |= O_RDONLY;

	// Creation/truncation flags
	if (flags & FS_CREATE)
		openFlags |= O_CREAT;
	if (flags & FS_TRUNCATE)
		openFlags |= O_TRUNC;
	if (flags & FS_APPEND)
		openFlags |= O_APPEND;

	SSIZE fd;
#if defined(PLATFORM_LINUX) && defined(ARCHITECTURE_AARCH64)
	fd = System::Call(SYS_OPENAT, AT_FDCWD, (USIZE)utf8Path, openFlags, mode);
#else
	fd = System::Call(SYS_OPEN, (USIZE)utf8Path, openFlags, mode);
#endif

	if (fd < 0)
		return Result<File, Error>::Err(Error::Posix((UINT32)(-fd)), Error::Fs_OpenFailed);

	return Result<File, Error>::Ok(File((PVOID)fd, 0));
}

Result<void, Error> FileSystem::Delete(PCWCHAR path)
{
	CHAR utf8Path[1024];
	NormalizePathToUtf8(path, Span<CHAR>(utf8Path));

#if defined(PLATFORM_LINUX) && defined(ARCHITECTURE_AARCH64)
	SSIZE result = System::Call(SYS_UNLINKAT, AT_FDCWD, (USIZE)utf8Path, 0);
#else
	SSIZE result = System::Call(SYS_UNLINK, (USIZE)utf8Path);
#endif
	if (result == 0)
		return Result<void, Error>::Ok();
	return Result<void, Error>::Err(Error::Posix((UINT32)(-result)), Error::Fs_DeleteFailed);
}

Result<void, Error> FileSystem::Exists(PCWCHAR path)
{
	CHAR utf8Path[1024];
	NormalizePathToUtf8(path, Span<CHAR>(utf8Path));

	UINT8 statbuf[144];

#if defined(PLATFORM_LINUX) && defined(ARCHITECTURE_AARCH64)
	SSIZE result = System::Call(SYS_FSTATAT, AT_FDCWD, (USIZE)utf8Path, (USIZE)statbuf, 0);
#elif defined(PLATFORM_MACOS)
	SSIZE result = System::Call(SYS_STAT64, (USIZE)utf8Path, (USIZE)statbuf);
#else
	SSIZE result = System::Call(SYS_STAT, (USIZE)utf8Path, (USIZE)statbuf);
#endif
	if (result == 0)
		return Result<void, Error>::Ok();
	return Result<void, Error>::Err(Error::Posix((UINT32)(-result)), Error::Fs_OpenFailed);
}

Result<void, Error> FileSystem::CreateDirectory(PCWCHAR path)
{
	CHAR utf8Path[1024];
	NormalizePathToUtf8(path, Span<CHAR>(utf8Path));

	// Mode 0755 (rwxr-xr-x)
	INT32 mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

#if defined(PLATFORM_LINUX) && defined(ARCHITECTURE_AARCH64)
	SSIZE result = System::Call(SYS_MKDIRAT, AT_FDCWD, (USIZE)utf8Path, mode);
#else
	SSIZE result = System::Call(SYS_MKDIR, (USIZE)utf8Path, mode);
#endif
	if (result == 0 || result == -17) // -EEXIST: directory already exists
		return Result<void, Error>::Ok();
	return Result<void, Error>::Err(Error::Posix((UINT32)(-result)), Error::Fs_CreateDirFailed);
}

Result<void, Error> FileSystem::DeleteDirectory(PCWCHAR path)
{
	CHAR utf8Path[1024];
	NormalizePathToUtf8(path, Span<CHAR>(utf8Path));

#if defined(PLATFORM_LINUX) && defined(ARCHITECTURE_AARCH64)
	SSIZE result = System::Call(SYS_UNLINKAT, AT_FDCWD, (USIZE)utf8Path, AT_REMOVEDIR);
#else
	SSIZE result = System::Call(SYS_RMDIR, (USIZE)utf8Path);
#endif
	if (result == 0)
		return Result<void, Error>::Ok();
	return Result<void, Error>::Err(Error::Posix((UINT32)(-result)), Error::Fs_DeleteDirFailed);
}
