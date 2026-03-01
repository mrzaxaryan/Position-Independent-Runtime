/**
 * directory_iterator.cc - UEFI DirectoryIterator Implementation
 *
 * Implements directory iteration using EFI_FILE_PROTOCOL.
 */

#include "platform/io/file_system/directory_iterator.h"
#include "platform/os/uefi/common/efi_context.h"
#include "platform/os/uefi/common/efi_file_protocol.h"
#include "core/memory/memory.h"
#include "platform/io/file_system/path.h"

// =============================================================================
// Helper: Build EFI GUIDs on the stack (no .rdata)
// =============================================================================

// EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID {964E5B22-6459-11D2-8E39-00A0C969723B}
static NOINLINE EFI_GUID MakeFsProtocolGuid()
{
	EFI_GUID g;
	g.Data1 = 0x964E5B22;
	g.Data2 = 0x6459;
	g.Data3 = 0x11D2;
	g.Data4[0] = 0x8E; g.Data4[1] = 0x39; g.Data4[2] = 0x00; g.Data4[3] = 0xA0;
	g.Data4[4] = 0xC9; g.Data4[5] = 0x69; g.Data4[6] = 0x72; g.Data4[7] = 0x3B;
	return g;
}

// =============================================================================
// Helper: Get Root Directory Handle
// =============================================================================

static EFI_FILE_PROTOCOL *GetRootDirectory()
{
	EFI_CONTEXT *ctx = GetEfiContext();
	if (ctx == nullptr || ctx->SystemTable == nullptr)
		return nullptr;

	EFI_BOOT_SERVICES *bs = ctx->SystemTable->BootServices;

	EFI_GUID FsGuid = MakeFsProtocolGuid();

	USIZE HandleCount = 0;
	EFI_HANDLE *HandleBuffer = nullptr;

	if (EFI_ERROR_CHECK(bs->LocateHandleBuffer(ByProtocol, &FsGuid, nullptr, &HandleCount, &HandleBuffer)) || HandleCount == 0)
		return nullptr;

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = nullptr;
	EFI_FILE_PROTOCOL *Root = nullptr;

	// Try each handle until we find a working filesystem
	for (USIZE i = 0; i < HandleCount; i++)
	{
		if (EFI_ERROR_CHECK(bs->OpenProtocol(HandleBuffer[i], &FsGuid, (PVOID *)&FileSystem, ctx->ImageHandle, nullptr, EFI_OPEN_PROTOCOL_GET_PROTOCOL)))
			continue;

		if (FileSystem != nullptr && !EFI_ERROR_CHECK(FileSystem->OpenVolume(FileSystem, &Root)))
		{
			bs->FreePool(HandleBuffer);
			return Root;
		}
	}

	bs->FreePool(HandleBuffer);
	return nullptr;
}

// =============================================================================
// Helper: Open File by Path from Root
// =============================================================================

static EFI_FILE_PROTOCOL *OpenFileFromRoot(EFI_FILE_PROTOCOL *Root, PCWCHAR path, UINT64 mode, UINT64 attributes)
{
	if (Root == nullptr || path == nullptr)
		return nullptr;

	// Normalize path separators (convert '/' to '\' for UEFI)
	WCHAR normalizedBuf[512];
	if (!Path::NormalizePath(path, Span<WCHAR>(normalizedBuf)))
		return nullptr;

	EFI_FILE_PROTOCOL *FileHandle = nullptr;
	EFI_STATUS Status = Root->Open(Root, &FileHandle, (CHAR16 *)normalizedBuf, mode, attributes);

	if (EFI_ERROR_CHECK(Status))
		return nullptr;

	return FileHandle;
}

// =============================================================================
// DirectoryIterator Class Implementation
// =============================================================================

DirectoryIterator::DirectoryIterator()
	: handle(nullptr), currentEntry{}, first(true)
{}

Result<DirectoryIterator, Error> DirectoryIterator::Create(PCWCHAR path)
{
	DirectoryIterator iter;
	(VOID) iter.first; // Suppress unused warning - UEFI uses Read to iterate

	EFI_FILE_PROTOCOL *Root = GetRootDirectory();
	if (Root == nullptr)
		return Result<DirectoryIterator, Error>::Err(Error::Fs_OpenFailed);

	// Empty path means root directory - use the volume root handle directly
	// rather than calling Open() with L"" which some firmware doesn't support
	if (path == nullptr || path[0] == 0)
	{
		iter.handle = (PVOID)Root;
		return Result<DirectoryIterator, Error>::Ok(static_cast<DirectoryIterator &&>(iter));
	}

	EFI_FILE_PROTOCOL *DirHandle = OpenFileFromRoot(Root, path, EFI_FILE_MODE_READ, 0);
	Root->Close(Root);

	if (DirHandle != nullptr)
	{
		iter.handle = (PVOID)DirHandle;
		return Result<DirectoryIterator, Error>::Ok(static_cast<DirectoryIterator &&>(iter));
	}
	return Result<DirectoryIterator, Error>::Err(Error::Fs_OpenFailed);
}

DirectoryIterator::DirectoryIterator(DirectoryIterator &&other) noexcept
	: handle(other.handle), currentEntry(other.currentEntry), first(other.first)
{
	other.handle = nullptr;
}

DirectoryIterator &DirectoryIterator::operator=(DirectoryIterator &&other) noexcept
{
	if (this != &other)
	{
		if (handle != nullptr)
		{
			EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)handle;
			fp->Close(fp);
		}
		handle = other.handle;
		currentEntry = other.currentEntry;
		first = other.first;
		other.handle = nullptr;
	}
	return *this;
}

DirectoryIterator::~DirectoryIterator()
{
	if (handle != nullptr)
	{
		EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)handle;
		fp->Close(fp);
		handle = nullptr;
	}
}

Result<void, Error> DirectoryIterator::Next()
{
	if (handle == nullptr)
		return Result<void, Error>::Err(Error::Fs_ReadFailed);

	EFI_FILE_PROTOCOL *fp = (EFI_FILE_PROTOCOL *)handle;

	// Allocate buffer for EFI_FILE_INFO (needs to include variable-length filename)
	// Use a fixed buffer size that should be large enough for most filenames
	UINT8 Buffer[512];
	USIZE BufferSize = sizeof(Buffer);

	EFI_STATUS Status = fp->Read(fp, &BufferSize, Buffer);

	if (EFI_ERROR_CHECK(Status))
		return Result<void, Error>::Err(Error::Uefi((UINT32)Status), Error::Fs_ReadFailed);

	// End of directory
	if (BufferSize == 0)
		return Result<void, Error>::Err(Error::Fs_ReadFailed);

	EFI_FILE_INFO *FileInfo = (EFI_FILE_INFO *)Buffer;

	// Copy filename to currentEntry
	INT32 i = 0;
	while (FileInfo->FileName[i] != 0 && i < 255)
	{
		currentEntry.Name[i] = FileInfo->FileName[i];
		i++;
	}
	currentEntry.Name[i] = 0;

	// Fill other fields
	currentEntry.Size = FileInfo->FileSize;
	currentEntry.IsDirectory = (FileInfo->Attribute & EFI_FILE_DIRECTORY) != 0;
	currentEntry.IsDrive = false;
	currentEntry.IsHidden = (FileInfo->Attribute & EFI_FILE_HIDDEN) != 0;
	currentEntry.IsSystem = (FileInfo->Attribute & EFI_FILE_SYSTEM) != 0;
	currentEntry.IsReadOnly = (FileInfo->Attribute & EFI_FILE_READ_ONLY) != 0;
	currentEntry.Type = 0;
	currentEntry.CreationTime = 0;
	currentEntry.LastModifiedTime = 0;

	return Result<void, Error>::Ok();
}

BOOL DirectoryIterator::IsValid() const
{
	return handle != nullptr;
}
