#include "platform/io/file_system/file.h"
#include "platform/io/logger.h"
#include "core/types/primitives.h"
#include "platform/os/windows/common/windows_types.h"
#include "platform/os/windows/common/ntdll.h"

// --- Internal Constructor (trivial — never fails) ---
File::File(PVOID handle, USIZE size) : fileHandle(handle), fileSize(size) {}

// --- Move Semantics ---
File::File(File &&other) noexcept : fileHandle(nullptr), fileSize(0)
{
	fileHandle = other.fileHandle;
	fileSize = other.fileSize;
	other.fileHandle = nullptr;
	other.fileSize = 0;
}

// Operator move assignment
File &File::operator=(File &&other) noexcept
{
	if (this != &other)
	{
		Close(); // Close existing handle before taking new one
		fileHandle = other.fileHandle;
		fileSize = other.fileSize;
		other.fileHandle = nullptr;
		other.fileSize = 0;
	}
	return *this;
}

// --- Logic ---
BOOL File::IsValid() const
{
	// Windows returns INVALID_HANDLE_VALUE (-1) on many errors,
	// but some APIs return nullptr. We check for both.
	return fileHandle != nullptr && fileHandle != INVALID_HANDLE_VALUE;
}

// Close the file handle
void File::Close()
{
	if (IsValid())
	{
		(void)NTDLL::ZwClose((PVOID)fileHandle);
		fileHandle = nullptr;
		fileSize = 0;
	}
}

// Read data from the file into the buffer
Result<UINT32, Error> File::Read(Span<UINT8> buffer)
{
	if (!IsValid())
		return Result<UINT32, Error>::Err(Error::Fs_ReadFailed);

	IO_STATUS_BLOCK ioStatusBlock;
	Memory::Zero(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));

	auto readResult = NTDLL::ZwReadFile((PVOID)fileHandle, nullptr, nullptr, nullptr, &ioStatusBlock, buffer.Data(), (UINT32)buffer.Size(), nullptr, nullptr);

	if (readResult)
	{
		return Result<UINT32, Error>::Ok((UINT32)ioStatusBlock.Information);
	}
	return Result<UINT32, Error>::Err(readResult, Error::Fs_ReadFailed);
}

// Write data from the buffer to the file
Result<UINT32, Error> File::Write(Span<const UINT8> buffer)
{
	if (!IsValid())
		return Result<UINT32, Error>::Err(Error::Fs_WriteFailed);

	IO_STATUS_BLOCK ioStatusBlock;
	Memory::Zero(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));

	auto writeResult = NTDLL::ZwWriteFile((PVOID)fileHandle, nullptr, nullptr, nullptr, &ioStatusBlock, (PVOID)buffer.Data(), (UINT32)buffer.Size(), nullptr, nullptr);

	if (writeResult)
	{
		return Result<UINT32, Error>::Ok((UINT32)ioStatusBlock.Information);
	}
	return Result<UINT32, Error>::Err(writeResult, Error::Fs_WriteFailed);
}

// Get the current file offset
USIZE File::GetOffset() const
{
	if (!IsValid())
		return 0;

	FILE_POSITION_INFORMATION posFile;
	IO_STATUS_BLOCK ioStatusBlock;
	Memory::Zero(&posFile, sizeof(posFile));
	Memory::Zero(&ioStatusBlock, sizeof(ioStatusBlock));

	auto queryResult = NTDLL::ZwQueryInformationFile((PVOID)fileHandle, &ioStatusBlock, &posFile, sizeof(posFile), FilePositionInformation);

	if (queryResult)
	{
		return (USIZE)posFile.CurrentByteOffset.QuadPart;
	}
	return 0;
}

// Set the file offset to an absolute position
void File::SetOffset(USIZE absoluteOffset)
{
	if (!IsValid())
		return;

	FILE_POSITION_INFORMATION posInfo;
	IO_STATUS_BLOCK ioStatusBlock;
	Memory::Zero(&posInfo, sizeof(FILE_POSITION_INFORMATION));
	Memory::Zero(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));
	posInfo.CurrentByteOffset.QuadPart = (INT64)absoluteOffset;
	// Set the file pointer to the specified absolute offset using ZwSetInformationFile
	(void)NTDLL::ZwSetInformationFile((PVOID)fileHandle, &ioStatusBlock, &posInfo, sizeof(posInfo), FilePositionInformation);
}

// Move the file offset by a relative amount based on the specified origin
void File::MoveOffset(SSIZE relativeAmount, OffsetOrigin origin)
{
	if (!IsValid())
		return;

	IO_STATUS_BLOCK ioStatusBlock;
	FILE_POSITION_INFORMATION posInfo;
	FILE_STANDARD_INFORMATION fileStandardInfo;
	Memory::Zero(&ioStatusBlock, sizeof(IO_STATUS_BLOCK));
	Memory::Zero(&posInfo, sizeof(FILE_POSITION_INFORMATION));
	Memory::Zero(&fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION));
	INT64 distance = 0;

	auto queryResult = NTDLL::ZwQueryInformationFile((PVOID)fileHandle, &ioStatusBlock, &posInfo, sizeof(posInfo), FilePositionInformation);
	if (!queryResult)
		return;

	switch (origin)
	{
	case OffsetOrigin::Start:
		distance = relativeAmount;
		break;
	case OffsetOrigin::Current:
		distance = posInfo.CurrentByteOffset.QuadPart + relativeAmount;
		break;
	case OffsetOrigin::End:
		queryResult = NTDLL::ZwQueryInformationFile(fileHandle, &ioStatusBlock, &fileStandardInfo, sizeof(fileStandardInfo), FileStandardInformation);
		if (!queryResult)
			return;
		distance = fileStandardInfo.EndOfFile.QuadPart + relativeAmount;
		break;
	default:
		LOG_ERROR("Invalid OffsetOrigin specified in MoveOffset");
		return;
	}
	posInfo.CurrentByteOffset.QuadPart = distance;
	(void)NTDLL::ZwSetInformationFile((PVOID)fileHandle, &ioStatusBlock, &posInfo, sizeof(posInfo), FilePositionInformation);
}
