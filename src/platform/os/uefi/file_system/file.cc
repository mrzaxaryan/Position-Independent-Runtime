/**
 * file.cc - UEFI File Implementation
 *
 * Implements File class operations using EFI_FILE_PROTOCOL.
 */

#include "platform/io/file_system/file.h"
#include "platform/os/uefi/common/efi_context.h"
#include "platform/os/uefi/common/efi_file_protocol.h"
#include "core/memory/memory.h"

// =============================================================================
// Helper: Build EFI GUIDs on the stack (no .rdata)
// =============================================================================

// EFI_FILE_INFO_ID {09576E92-6D3F-11D2-8E39-00A0C969723B}
static NOINLINE EFI_GUID MakeFileInfoGuid()
{
	EFI_GUID g;
	g.Data1 = 0x09576E92;
	g.Data2 = 0x6D3F;
	g.Data3 = 0x11D2;
	g.Data4[0] = 0x8E; g.Data4[1] = 0x39; g.Data4[2] = 0x00; g.Data4[3] = 0xA0;
	g.Data4[4] = 0xC9; g.Data4[5] = 0x69; g.Data4[6] = 0x72; g.Data4[7] = 0x3B;
	return g;
}

// =============================================================================
// File Class Implementation
// =============================================================================

// --- Internal Constructor (trivial — never fails) ---
File::File(PVOID handle, USIZE size) : fileHandle(handle), fileSize(size) {}

BOOL File::IsValid() const
{
	return fileHandle != nullptr;
}

VOID File::Close()
{
	if (fileHandle != nullptr)
	{
		EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)fileHandle;
		fp->Close(fp);
		fileHandle = nullptr;
	}
	fileSize = 0;
}

Result<UINT32, Error> File::Read(Span<UINT8> buffer)
{
	if (fileHandle == nullptr || buffer.Data() == nullptr || buffer.Size() == 0)
		return Result<UINT32, Error>::Err(Error::Fs_ReadFailed);

	EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)fileHandle;
	USIZE readSize = buffer.Size();

	EFI_STATUS Status = fp->Read(fp, &readSize, buffer.Data());
	if (EFI_ERROR_CHECK(Status))
		return Result<UINT32, Error>::Err(Error::Uefi((UINT32)Status), Error::Fs_ReadFailed);

	return Result<UINT32, Error>::Ok((UINT32)readSize);
}

Result<UINT32, Error> File::Write(Span<const UINT8> buffer)
{
	if (fileHandle == nullptr || buffer.Data() == nullptr || buffer.Size() == 0)
		return Result<UINT32, Error>::Err(Error::Fs_WriteFailed);

	EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)fileHandle;
	USIZE writeSize = buffer.Size();

	EFI_STATUS Status = fp->Write(fp, &writeSize, (PVOID)buffer.Data());
	if (EFI_ERROR_CHECK(Status))
		return Result<UINT32, Error>::Err(Error::Uefi((UINT32)Status), Error::Fs_WriteFailed);

	// Update file size if we wrote past the end
	UINT64 pos = 0;
	fp->GetPosition(fp, &pos);
	if (pos > fileSize)
		fileSize = pos;

	return Result<UINT32, Error>::Ok((UINT32)writeSize);
}

USIZE File::GetOffset() const
{
	if (fileHandle == nullptr)
		return 0;

	EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)fileHandle;
	UINT64 position = 0;
	fp->GetPosition(fp, &position);
	return (USIZE)position;
}

VOID File::SetOffset(USIZE absoluteOffset)
{
	if (fileHandle == nullptr)
		return;

	EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)fileHandle;
	fp->SetPosition(fp, absoluteOffset);
}

VOID File::MoveOffset(SSIZE relativeAmount, OffsetOrigin origin)
{
	if (fileHandle == nullptr)
		return;

	EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)fileHandle;
	UINT64 newPosition = 0;

	switch (origin)
	{
	case OffsetOrigin::Start:
		newPosition = (relativeAmount >= 0) ? (UINT64)relativeAmount : 0;
		break;
	case OffsetOrigin::Current:
	{
		UINT64 currentPos = 0;
		fp->GetPosition(fp, &currentPos);
		if (relativeAmount >= 0)
			newPosition = currentPos + relativeAmount;
		else
			newPosition = (currentPos > (UINT64)(-relativeAmount)) ? currentPos + relativeAmount : 0;
	}
	break;
	case OffsetOrigin::End:
		if (relativeAmount >= 0)
			newPosition = fileSize + relativeAmount;
		else
			newPosition = (fileSize > (UINT64)(-relativeAmount)) ? fileSize + relativeAmount : 0;
		break;
	}

	fp->SetPosition(fp, newPosition);
}

File::File(File &&other) noexcept
	: fileHandle(other.fileHandle), fileSize(other.fileSize)
{
	other.fileHandle = nullptr;
	other.fileSize = 0;
}

File &File::operator=(File &&other) noexcept
{
	if (this != &other)
	{
		Close();
		fileHandle = other.fileHandle;
		fileSize = other.fileSize;
		other.fileHandle = nullptr;
		other.fileSize = 0;
	}
	return *this;
}
