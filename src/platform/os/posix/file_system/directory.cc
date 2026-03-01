#include "platform/io/file_system/directory.h"
#include "platform/io/file_system/path.h"
#include "core/memory/memory.h"
#include "core/string/string.h"
#include "core/encoding/utf16.h"
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

// =============================================================================
// Helper: Normalize a wide path to a null-terminated UTF-8 string.
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
// Directory Implementation
// =============================================================================

Result<void, Error> Directory::Create(PCWCHAR path)
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

Result<void, Error> Directory::Delete(PCWCHAR path)
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
