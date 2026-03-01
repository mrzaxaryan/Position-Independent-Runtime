#include "platform/io/file_system/file_system.h"
#include "platform/io/logger.h"
#include "core/types/primitives.h"
#include "core/string/string.h"
#include "platform/os/windows/common/windows_types.h"
#include "platform/os/windows/common/ntdll.h"

// --- FileSystem Implementation ---
Result<File, Error> FileSystem::Open(PCWCHAR path, INT32 flags)
{
	UINT32 dwDesiredAccess = 0;
	UINT32 dwShareMode = FILE_SHARE_READ;
	UINT32 dwCreationDisposition = FILE_OPEN;
	UINT32 ntFlags = 0;
	UINT32 fileAttributes = FILE_ATTRIBUTE_NORMAL;

	// 1. Map Access Flags
	if (flags & FS_READ)
		dwDesiredAccess |= GENERIC_READ;
	if (flags & FS_WRITE)
		dwDesiredAccess |= GENERIC_WRITE;
	if (flags & FS_APPEND)
		dwDesiredAccess |= FILE_APPEND_DATA;

	// 2. Map Creation/Truncation Flags
	if (flags & FS_CREATE)
	{
		if (flags & FS_TRUNCATE)
			dwCreationDisposition = FILE_OVERWRITE_IF;
		else
			dwCreationDisposition = FILE_OPEN_IF;
	}
	else if (flags & FS_TRUNCATE)
	{
		dwCreationDisposition = FILE_OVERWRITE;
	}

	// Synchronous I/O — PIR never uses overlapped file handles
	ntFlags |= FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE;

	// Always allow waiting and querying attributes
	dwDesiredAccess |= SYNCHRONIZE | FILE_READ_ATTRIBUTES;

	// Convert DOS path to NT path
	UNICODE_STRING ntPathU;
	if (!NTDLL::RtlDosPathNameToNtPathName_U(path, &ntPathU, nullptr, nullptr))
		return Result<File, Error>::Err(Error::Fs_PathResolveFailed, Error::Fs_OpenFailed);

	OBJECT_ATTRIBUTES objAttr;
	InitializeObjectAttributes(&objAttr, &ntPathU, 0, nullptr, nullptr);

	IO_STATUS_BLOCK ioStatusBlock;
	PVOID hFile = nullptr;

	auto createResult = NTDLL::ZwCreateFile(
		&hFile,
		dwDesiredAccess,
		&objAttr,
		&ioStatusBlock,
		nullptr,
		fileAttributes,
		dwShareMode,
		dwCreationDisposition,
		ntFlags,
		nullptr,
		0);

	NTDLL::RtlFreeUnicodeString(&ntPathU);

	if (!createResult || hFile == INVALID_HANDLE_VALUE)
		return Result<File, Error>::Err(createResult, Error::Fs_OpenFailed);

	// Query file size before constructing the File (keeps the constructor trivial)
	USIZE size = 0;
	FILE_STANDARD_INFORMATION fileStandardInfo;
	IO_STATUS_BLOCK sizeIoBlock;
	Memory::Zero(&fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION));
	Memory::Zero(&sizeIoBlock, sizeof(IO_STATUS_BLOCK));
	auto sizeResult = NTDLL::ZwQueryInformationFile(hFile, &sizeIoBlock, &fileStandardInfo, sizeof(fileStandardInfo), FileStandardInformation);
	if (sizeResult)
		size = fileStandardInfo.EndOfFile.QuadPart;

	return Result<File, Error>::Ok(File((PVOID)hFile, size));
}

// Delete a file at the specified path
Result<void, Error> FileSystem::Delete(PCWCHAR path)
{
	UNICODE_STRING ntName;
	OBJECT_ATTRIBUTES attr;
	PVOID hFile = nullptr;
	IO_STATUS_BLOCK io;

	if (!NTDLL::RtlDosPathNameToNtPathName_U(path, &ntName, nullptr, nullptr))
		return Result<void, Error>::Err(Error::Fs_PathResolveFailed, Error::Fs_DeleteFailed);

	InitializeObjectAttributes(&attr, &ntName, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

	auto createResult = NTDLL::ZwCreateFile(&hFile, SYNCHRONIZE | DELETE, &attr, &io, nullptr, 0,
											FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
											FILE_OPEN, FILE_DELETE_ON_CLOSE | FILE_NON_DIRECTORY_FILE, nullptr, 0);

	if (!createResult)
	{
		NTDLL::RtlFreeUnicodeString(&ntName);
		return Result<void, Error>::Err(createResult, Error::Fs_DeleteFailed);
	}

	(void)NTDLL::ZwClose(hFile);
	NTDLL::RtlFreeUnicodeString(&ntName);
	return Result<void, Error>::Ok();
}

// Check if a file exists at the specified path
Result<void, Error> FileSystem::Exists(PCWCHAR path)
{
	OBJECT_ATTRIBUTES objAttr;
	UNICODE_STRING uniName;
	FILE_BASIC_INFORMATION fileBasicInfo;

	if (!NTDLL::RtlDosPathNameToNtPathName_U(path, &uniName, nullptr, nullptr))
		return Result<void, Error>::Err(Error::Fs_PathResolveFailed);

	InitializeObjectAttributes(&objAttr, &uniName, 0, nullptr, nullptr);
	auto queryResult = NTDLL::ZwQueryAttributesFile(&objAttr, &fileBasicInfo);

	NTDLL::RtlFreeUnicodeString(&uniName);

	if (!queryResult)
		return Result<void, Error>::Err(queryResult, Error::Fs_OpenFailed);

	if (fileBasicInfo.FileAttributes == 0xFFFFFFFF)
		return Result<void, Error>::Err(Error::Fs_OpenFailed);

	return Result<void, Error>::Ok();
}

// --- FileSystem Directory Management ---
Result<void, Error> FileSystem::CreateDirectory(PCWCHAR path)
{
	// Returns non-zero on success
	PVOID hDir;
	UNICODE_STRING uniName;
	OBJECT_ATTRIBUTES objAttr;
	IO_STATUS_BLOCK ioStatusBlock;

	if (!NTDLL::RtlDosPathNameToNtPathName_U(path, &uniName, nullptr, nullptr))
		return Result<void, Error>::Err(Error::Fs_PathResolveFailed, Error::Fs_CreateDirFailed);

	InitializeObjectAttributes(&objAttr, &uniName, 0, nullptr, nullptr);

	auto createResult = NTDLL::ZwCreateFile(
		&hDir,
		FILE_LIST_DIRECTORY | SYNCHRONIZE,
		&objAttr,
		&ioStatusBlock,
		nullptr,
		FILE_ATTRIBUTE_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		nullptr,
		0);

	NTDLL::RtlFreeUnicodeString(&uniName);

	if (createResult)
	{
		(void)NTDLL::ZwClose(hDir);
		return Result<void, Error>::Ok();
	}
	LOG_ERROR("CreateDirectory failed: errors=%e path=%ls", createResult.Error(), path);
	return Result<void, Error>::Err(createResult, Error::Fs_CreateDirFailed);
}

// Delete a directory at the specified path
Result<void, Error> FileSystem::DeleteDirectory(PCWCHAR path)
{
	PVOID hDir;
	FILE_DISPOSITION_INFORMATION disp;
	UNICODE_STRING uniName;
	OBJECT_ATTRIBUTES objAttr;
	IO_STATUS_BLOCK ioStatusBlock;
	Memory::Zero(&disp, sizeof(FILE_DISPOSITION_INFORMATION));
	disp.DeleteFile = true;

	if (!NTDLL::RtlDosPathNameToNtPathName_U(path, &uniName, nullptr, nullptr))
		return Result<void, Error>::Err(Error::Fs_PathResolveFailed, Error::Fs_DeleteDirFailed);

	InitializeObjectAttributes(&objAttr, &uniName, 0, nullptr, nullptr);

	auto openResult = NTDLL::ZwOpenFile(&hDir, DELETE | SYNCHRONIZE, &objAttr, &ioStatusBlock, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
	if (!openResult)
	{
		NTDLL::RtlFreeUnicodeString(&uniName);
		return Result<void, Error>::Err(openResult, Error::Fs_DeleteDirFailed);
	}

	auto setResult = NTDLL::ZwSetInformationFile(
		hDir,
		&ioStatusBlock,
		&disp,
		sizeof(disp),
		FileDispositionInformation);

	(void)NTDLL::ZwClose(hDir);
	NTDLL::RtlFreeUnicodeString(&uniName);

	if (!setResult)
		return Result<void, Error>::Err(setResult, Error::Fs_DeleteDirFailed);

	return Result<void, Error>::Ok();
}
