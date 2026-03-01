/**
 * file_system.cc - UEFI FileSystem Implementation
 *
 * Implements FileSystem static methods using EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
 * and EFI_FILE_PROTOCOL.
 */

#include "platform/io/file_system/file_system.h"
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
// Helper: Query file size via EFI_FILE_INFO
// =============================================================================

static NOINLINE USIZE QueryFileSize(EFI_FILE_PROTOCOL &fp)
{
	EFI_GUID FileInfoId = MakeFileInfoGuid();
	USIZE InfoSize = 0;
	fp.GetInfo(&fp, &FileInfoId, &InfoSize, nullptr);
	if (InfoSize == 0)
		return 0;

	EFI_CONTEXT *ctx = GetEfiContext();
	EFI_BOOT_SERVICES *bs = ctx->SystemTable->BootServices;

	USIZE size = 0;
	EFI_FILE_INFO *FileInfo = nullptr;
	if (!EFI_ERROR_CHECK(bs->AllocatePool(EfiLoaderData, InfoSize, (PVOID *)&FileInfo)))
	{
		if (!EFI_ERROR_CHECK(fp.GetInfo(&fp, &FileInfoId, &InfoSize, FileInfo)))
			size = FileInfo->FileSize;
		bs->FreePool(FileInfo);
	}
	return size;
}

// =============================================================================
// Helper: Truncate file to zero length via EFI_FILE_INFO
// =============================================================================

static NOINLINE VOID TruncateFile(EFI_FILE_PROTOCOL &fp)
{
	EFI_GUID FileInfoId = MakeFileInfoGuid();
	USIZE InfoSize = 0;
	fp.GetInfo(&fp, &FileInfoId, &InfoSize, nullptr);
	if (InfoSize == 0)
		return;

	EFI_CONTEXT *ctx = GetEfiContext();
	EFI_BOOT_SERVICES *bs = ctx->SystemTable->BootServices;

	EFI_FILE_INFO *FileInfo = nullptr;
	if (!EFI_ERROR_CHECK(bs->AllocatePool(EfiLoaderData, InfoSize, (PVOID *)&FileInfo)))
	{
		if (!EFI_ERROR_CHECK(fp.GetInfo(&fp, &FileInfoId, &InfoSize, FileInfo)))
		{
			FileInfo->FileSize = 0;
			fp.SetInfo(&fp, &FileInfoId, InfoSize, FileInfo);
		}
		bs->FreePool(FileInfo);
	}
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
// FileSystem Class Implementation
// =============================================================================

Result<File, Error> FileSystem::Open(PCWCHAR path, INT32 flags)
{
	EFI_FILE_PROTOCOL *Root = GetRootDirectory();
	if (Root == nullptr)
		return Result<File, Error>::Err(Error::Fs_OpenFailed);

	// Convert flags to EFI modes
	UINT64 mode = 0;
	UINT64 attributes = 0;

	if (flags & FS_READ)
		mode |= EFI_FILE_MODE_READ;
	if (flags & FS_WRITE)
		mode |= EFI_FILE_MODE_WRITE;
	if (flags & FS_CREATE)
		mode |= EFI_FILE_MODE_CREATE;

	// If no mode specified, default to read
	if (mode == 0)
		mode = EFI_FILE_MODE_READ;

	// Create mode requires write mode
	if (mode & EFI_FILE_MODE_CREATE)
		mode |= EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;

	EFI_FILE_PROTOCOL *FileHandle = OpenFileFromRoot(Root, path, mode, attributes);
	Root->Close(Root);

	if (FileHandle == nullptr)
		return Result<File, Error>::Err(Error::Fs_OpenFailed);

	// Handle truncate flag
	if (flags & FS_TRUNCATE)
		TruncateFile(*FileHandle);

	// Query file size before constructing the File (keeps the constructor trivial)
	USIZE size = QueryFileSize(*FileHandle);

	return Result<File, Error>::Ok(File((PVOID)FileHandle, size));
}

Result<void, Error> FileSystem::Delete(PCWCHAR path)
{
	EFI_FILE_PROTOCOL *Root = GetRootDirectory();
	if (Root == nullptr)
		return Result<void, Error>::Err(Error::Fs_DeleteFailed);

	EFI_FILE_PROTOCOL *FileHandle = OpenFileFromRoot(Root, path, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
	Root->Close(Root);

	if (FileHandle == nullptr)
		return Result<void, Error>::Err(Error::Fs_DeleteFailed);

	// EFI_FILE_PROTOCOL.Delete closes the handle and deletes the file
	EFI_STATUS Status = FileHandle->Delete(FileHandle);
	if (EFI_ERROR_CHECK(Status))
		return Result<void, Error>::Err(Error::Uefi((UINT32)Status), Error::Fs_DeleteFailed);
	return Result<void, Error>::Ok();
}

Result<void, Error> FileSystem::Exists(PCWCHAR path)
{
	EFI_FILE_PROTOCOL *Root = GetRootDirectory();
	if (Root == nullptr)
		return Result<void, Error>::Err(Error::Fs_OpenFailed);

	EFI_FILE_PROTOCOL *FileHandle = OpenFileFromRoot(Root, path, EFI_FILE_MODE_READ, 0);
	Root->Close(Root);

	if (FileHandle == nullptr)
		return Result<void, Error>::Err(Error::Fs_OpenFailed);

	FileHandle->Close(FileHandle);
	return Result<void, Error>::Ok();
}

Result<void, Error> FileSystem::CreateDirectory(PCWCHAR path)
{
	EFI_FILE_PROTOCOL *Root = GetRootDirectory();
	if (Root == nullptr)
		return Result<void, Error>::Err(Error::Fs_CreateDirFailed);

	// Normalize path separators (convert '/' to '\' for UEFI)
	WCHAR normalizedBuf[512];
	if (!Path::NormalizePath(path, Span<WCHAR>(normalizedBuf)))
	{
		Root->Close(Root);
		return Result<void, Error>::Err(Error::Fs_PathResolveFailed, Error::Fs_CreateDirFailed);
	}

	EFI_FILE_PROTOCOL *DirHandle = nullptr;
	EFI_STATUS Status = Root->Open(Root, &DirHandle, (CHAR16 *)normalizedBuf,
								   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
								   EFI_FILE_DIRECTORY);
	Root->Close(Root);

	if (EFI_ERROR_CHECK(Status) || DirHandle == nullptr)
		return Result<void, Error>::Err(Error::Uefi((UINT32)Status), Error::Fs_CreateDirFailed);

	DirHandle->Close(DirHandle);
	return Result<void, Error>::Ok();
}

Result<void, Error> FileSystem::DeleteDirectory(PCWCHAR path)
{
	EFI_FILE_PROTOCOL *Root = GetRootDirectory();
	if (Root == nullptr)
		return Result<void, Error>::Err(Error::Fs_DeleteDirFailed);

	EFI_FILE_PROTOCOL *DirHandle = OpenFileFromRoot(Root, path, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
	Root->Close(Root);

	if (DirHandle == nullptr)
		return Result<void, Error>::Err(Error::Fs_DeleteDirFailed);

	// EFI_FILE_PROTOCOL.Delete works for both files and directories
	EFI_STATUS Status = DirHandle->Delete(DirHandle);
	if (EFI_ERROR_CHECK(Status))
		return Result<void, Error>::Err(Error::Uefi((UINT32)Status), Error::Fs_DeleteDirFailed);
	return Result<void, Error>::Ok();
}
