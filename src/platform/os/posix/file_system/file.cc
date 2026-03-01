#include "platform/io/file_system/file.h"
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

// --- Internal Constructor (trivial — never fails) ---
File::File(PVOID handle, USIZE size) : fileHandle(handle), fileSize(size) {}

File::File(File &&other) noexcept : fileHandle(nullptr), fileSize(0)
{
	fileHandle = other.fileHandle;
	fileSize = other.fileSize;
	other.fileHandle = (PVOID)INVALID_FD;
	other.fileSize = 0;
}

File &File::operator=(File &&other) noexcept
{
	if (this != &other)
	{
		Close();
		fileHandle = other.fileHandle;
		fileSize = other.fileSize;
		other.fileHandle = (PVOID)INVALID_FD;
		other.fileSize = 0;
	}
	return *this;
}

BOOL File::IsValid() const
{
	SSIZE fd = (SSIZE)fileHandle;
	return fd >= 0;
}

VOID File::Close()
{
	if (IsValid())
	{
		System::Call(SYS_CLOSE, (USIZE)fileHandle);
		fileHandle = (PVOID)INVALID_FD;
		fileSize = 0;
	}
}

Result<UINT32, Error> File::Read(Span<UINT8> buffer)
{
	if (!IsValid())
		return Result<UINT32, Error>::Err(Error::Fs_ReadFailed);

	SSIZE result = System::Call(SYS_READ, (USIZE)fileHandle, (USIZE)buffer.Data(), buffer.Size());
	if (result >= 0)
		return Result<UINT32, Error>::Ok((UINT32)result);
	return Result<UINT32, Error>::Err(Error::Posix((UINT32)(-result)), Error::Fs_ReadFailed);
}

Result<UINT32, Error> File::Write(Span<const UINT8> buffer)
{
	if (!IsValid())
		return Result<UINT32, Error>::Err(Error::Fs_WriteFailed);

	SSIZE result = System::Call(SYS_WRITE, (USIZE)fileHandle, (USIZE)buffer.Data(), buffer.Size());
	if (result >= 0)
		return Result<UINT32, Error>::Ok((UINT32)result);
	return Result<UINT32, Error>::Err(Error::Posix((UINT32)(-result)), Error::Fs_WriteFailed);
}

USIZE File::GetOffset() const
{
	if (!IsValid())
		return 0;

	SSIZE result = System::Call(SYS_LSEEK, (USIZE)fileHandle, 0, SEEK_CUR);
	return (result >= 0) ? (USIZE)result : 0;
}

VOID File::SetOffset(USIZE absoluteOffset)
{
	if (!IsValid())
		return;

	System::Call(SYS_LSEEK, (USIZE)fileHandle, absoluteOffset, SEEK_SET);
}

VOID File::MoveOffset(SSIZE relativeAmount, OffsetOrigin origin)
{
	if (!IsValid())
		return;

	INT32 whence = SEEK_CUR;
	if (origin == OffsetOrigin::Start)
		whence = SEEK_SET;
	else if (origin == OffsetOrigin::End)
		whence = SEEK_END;

	System::Call(SYS_LSEEK, (USIZE)fileHandle, (USIZE)relativeAmount, whence);
}
