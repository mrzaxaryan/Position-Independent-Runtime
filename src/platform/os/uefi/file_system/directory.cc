/**
 * directory.cc - UEFI Directory Implementation
 *
 * Implements Directory static methods using EFI_FILE_PROTOCOL.
 */

#include "platform/io/file_system/directory.h"
#include "platform/io/file_system/path.h"
#include "platform/os/uefi/common/efi_context.h"
#include "platform/os/uefi/common/efi_file_protocol.h"
#include "core/memory/memory.h"

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
// Directory Implementation
// =============================================================================

Result<void, Error> Directory::Create(PCWCHAR path)
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

Result<void, Error> Directory::Delete(PCWCHAR path)
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
