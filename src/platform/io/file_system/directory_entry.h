#pragma once

#include "core/types/primitives.h"

#pragma pack(push, 1) // Ensure no padding in structures
// Directory entry structure
struct DirectoryEntry
{
	WCHAR name[256];         // File or directory name
	UINT64 creationTime;     // Filetime format
	UINT64 lastModifiedTime; // Filetime format
	UINT64 size;             // Size in bytes
	UINT32 type;             // Store DriveType (2=Removable, 3=Fixed, etc.)
	BOOL isDirectory;        // Set if the entry is a directory
	BOOL isDrive;            // Set if the entry represents a root (e.g., C:\)
	BOOL isHidden;           // Flag for hidden files
	BOOL isSystem;           // Flag for system files
	BOOL isReadOnly;         // Flag for read-only files
};

#pragma pack(pop)
