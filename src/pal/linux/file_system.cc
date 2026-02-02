#include "file_system.h"
#include "system.h"
#include "string.h"

// Linux syscall numbers for this file
#if defined(ARCHITECTURE_X86_64)
constexpr USIZE SYS_OPEN = 2;
constexpr USIZE SYS_CLOSE = 3;
constexpr USIZE SYS_READ = 0;
constexpr USIZE SYS_WRITE = 1;
constexpr USIZE SYS_LSEEK = 8;
constexpr USIZE SYS_STAT = 4;
constexpr USIZE SYS_UNLINK = 87;
constexpr USIZE SYS_MKDIR = 83;
constexpr USIZE SYS_RMDIR = 84;
constexpr USIZE SYS_GETDENTS64 = 217;
#elif defined(ARCHITECTURE_I386)
constexpr USIZE SYS_OPEN = 5;
constexpr USIZE SYS_CLOSE = 6;
constexpr USIZE SYS_READ = 3;
constexpr USIZE SYS_WRITE = 4;
constexpr USIZE SYS_LSEEK = 19;
constexpr USIZE SYS_STAT = 106;
constexpr USIZE SYS_UNLINK = 10;
constexpr USIZE SYS_MKDIR = 39;
constexpr USIZE SYS_RMDIR = 40;
constexpr USIZE SYS_GETDENTS64 = 220;
#elif defined(ARCHITECTURE_AARCH64)
constexpr USIZE SYS_OPENAT = 56;
constexpr USIZE SYS_CLOSE = 57;
constexpr USIZE SYS_READ = 63;
constexpr USIZE SYS_WRITE = 64;
constexpr USIZE SYS_LSEEK = 62;
constexpr USIZE SYS_FSTATAT = 79;
constexpr USIZE SYS_UNLINKAT = 35;
constexpr USIZE SYS_MKDIRAT = 34;
constexpr USIZE SYS_GETDENTS64 = 61;
constexpr SSIZE AT_FDCWD = -100;
constexpr INT32 AT_REMOVEDIR = 0x200;
#elif defined(ARCHITECTURE_ARMV7A)
constexpr USIZE SYS_OPEN = 5;
constexpr USIZE SYS_CLOSE = 6;
constexpr USIZE SYS_READ = 3;
constexpr USIZE SYS_WRITE = 4;
constexpr USIZE SYS_LSEEK = 19;
constexpr USIZE SYS_STAT = 106;
constexpr USIZE SYS_UNLINK = 10;
constexpr USIZE SYS_MKDIR = 39;
constexpr USIZE SYS_RMDIR = 40;
constexpr USIZE SYS_GETDENTS64 = 220;
#endif

// Linux open flags
constexpr INT32 O_RDONLY = 0x0000;
constexpr INT32 O_WRONLY = 0x0001;
constexpr INT32 O_RDWR = 0x0002;
constexpr INT32 O_CREAT = 0x0040;
constexpr INT32 O_TRUNC = 0x0200;
constexpr INT32 O_APPEND = 0x0400;
#if defined(ARCHITECTURE_X86_64) || defined(ARCHITECTURE_I386)
constexpr INT32 O_DIRECTORY = 0x10000;
#elif defined(ARCHITECTURE_AARCH64) || defined(ARCHITECTURE_ARMV7A)
constexpr INT32 O_DIRECTORY = 0x4000;
#endif

// Linux file modes
constexpr INT32 S_IRUSR = 0x0100;  // User read
constexpr INT32 S_IWUSR = 0x0080;  // User write
constexpr INT32 S_IRGRP = 0x0020;  // Group read
constexpr INT32 S_IWGRP = 0x0010;  // Group write
constexpr INT32 S_IROTH = 0x0004;  // Others read
constexpr INT32 S_IXUSR = 0x0040;  // User execute
constexpr INT32 S_IXGRP = 0x0008;  // Group execute
constexpr INT32 S_IXOTH = 0x0001;  // Others execute

// Directory entry types
constexpr UINT8 DT_DIR = 4;

// Invalid file descriptor
constexpr SSIZE INVALID_FD = -1;

// Linux dirent64 structure
struct linux_dirent64
{
    UINT64 d_ino;
    INT64 d_off;
    UINT16 d_reclen;
    UINT8 d_type;
    CHAR d_name[];
};

// --- File Implementation ---

File::File(PVOID handle) : fileHandle(handle), fileSize(0)
{
    // TODO: Get file size using fstat if needed
}

File::File(File&& other) noexcept : fileHandle(nullptr), fileSize(0)
{
    fileHandle = other.fileHandle;
    fileSize = other.fileSize;
    other.fileHandle = (PVOID)INVALID_FD;
    other.fileSize = 0;
}

File& File::operator=(File&& other) noexcept
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

UINT32 File::Read(PVOID buffer, UINT32 size)
{
    if (!IsValid())
        return 0;

    SSIZE result = System::Call(SYS_READ, (USIZE)fileHandle, (USIZE)buffer, size);
    return (result >= 0) ? (UINT32)result : 0;
}

UINT32 File::Write(const VOID* buffer, USIZE size)
{
    if (!IsValid())
        return 0;

    SSIZE result = System::Call(SYS_WRITE, (USIZE)fileHandle, (USIZE)buffer, size);
    return (result >= 0) ? (UINT32)result : 0;
}

USIZE File::GetOffset() const
{
    if (!IsValid())
        return 0;

    // lseek with offset 0 and SEEK_CUR returns current position
    SSIZE result = System::Call(SYS_LSEEK, (USIZE)fileHandle, 0, (USIZE)OffsetOrigin::Current);
    return (result >= 0) ? (USIZE)result : 0;
}

VOID File::SetOffset(USIZE absoluteOffset)
{
    if (!IsValid())
        return;

    System::Call(SYS_LSEEK, (USIZE)fileHandle, absoluteOffset, (USIZE)OffsetOrigin::Start);
}

VOID File::MoveOffset(SSIZE relativeAmount, OffsetOrigin origin)
{
    if (!IsValid())
        return;

    System::Call(SYS_LSEEK, (USIZE)fileHandle, (USIZE)relativeAmount, (USIZE)origin);
}

// --- FileSystem Implementation ---

File FileSystem::Open(PCWCHAR path, INT32 flags)
{
    // Convert wide char path to UTF-8
    CHAR utf8Path[1024];
    String::WideToUtf8(path, utf8Path, sizeof(utf8Path));

    // Map FileSystem flags to Linux open flags
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

    // Open the file
    SSIZE fd;
#if defined(ARCHITECTURE_X86_64) || defined(ARCHITECTURE_I386) || defined(ARCHITECTURE_ARMV7A)
    fd = System::Call(SYS_OPEN, (USIZE)utf8Path, openFlags, mode);
#else
    // ARM64 uses openat instead of open
    fd = System::Call(SYS_OPENAT, AT_FDCWD, (USIZE)utf8Path, openFlags, mode);
#endif

    if (fd < 0)
        return File();

    return File((PVOID)fd);
}

BOOL FileSystem::Delete(PCWCHAR path)
{
    CHAR utf8Path[1024];
    String::WideToUtf8(path, utf8Path, sizeof(utf8Path));

#if defined(ARCHITECTURE_AARCH64)
    return System::Call(SYS_UNLINKAT, AT_FDCWD, (USIZE)utf8Path, 0) == 0;
#else
    return System::Call(SYS_UNLINK, (USIZE)utf8Path) == 0;
#endif
}

BOOL FileSystem::Exists(PCWCHAR path)
{
    CHAR utf8Path[1024];
    String::WideToUtf8(path, utf8Path, sizeof(utf8Path));

    // stat buffer - we don't need the contents, just check if stat succeeds
    UINT8 statbuf[144];

#if defined(ARCHITECTURE_AARCH64)
    return System::Call(SYS_FSTATAT, AT_FDCWD, (USIZE)utf8Path, (USIZE)statbuf, 0) == 0;
#else
    return System::Call(SYS_STAT, (USIZE)utf8Path, (USIZE)statbuf) == 0;
#endif
}

BOOL FileSystem::CreateDirectroy(PCWCHAR path)
{
    CHAR utf8Path[1024];
    String::WideToUtf8(path, utf8Path, sizeof(utf8Path));

    // Mode 0755 (rwxr-xr-x)
    INT32 mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

#if defined(ARCHITECTURE_AARCH64)
    return System::Call(SYS_MKDIRAT, AT_FDCWD, (USIZE)utf8Path, mode) == 0;
#else
    return System::Call(SYS_MKDIR, (USIZE)utf8Path, mode) == 0;
#endif
}

BOOL FileSystem::DeleteDirectory(PCWCHAR path)
{
    CHAR utf8Path[1024];
    String::WideToUtf8(path, utf8Path, sizeof(utf8Path));

#if defined(ARCHITECTURE_AARCH64)
    return System::Call(SYS_UNLINKAT, AT_FDCWD, (USIZE)utf8Path, AT_REMOVEDIR) == 0;
#else
    return System::Call(SYS_RMDIR, (USIZE)utf8Path) == 0;
#endif
}

// --- DirectoryIterator Implementation ---

DirectoryIterator::DirectoryIterator(PCWCHAR path)
    : handle((PVOID)INVALID_FD), first(FALSE), nread(0), bpos(0)
{
    CHAR utf8Path[1024];

    // On Linux, if path is empty, we default to the current directory
    if (path && path[0] != L'\0')
    {
        String::WideToUtf8(path, utf8Path, sizeof(utf8Path));
    }
    else
    {
        utf8Path[0] = '.';
        utf8Path[1] = '\0';
    }

    // Open the directory with O_RDONLY | O_DIRECTORY
    SSIZE fd;
#if defined(ARCHITECTURE_X86_64) || defined(ARCHITECTURE_I386) || defined(ARCHITECTURE_ARMV7A)
    fd = System::Call(SYS_OPEN, (USIZE)utf8Path, O_RDONLY | O_DIRECTORY);
#else
    fd = System::Call(SYS_OPENAT, AT_FDCWD, (USIZE)utf8Path, O_RDONLY | O_DIRECTORY);
#endif

    if (fd >= 0)
    {
        handle = (PVOID)fd;
        first = TRUE;
    }
}

DirectoryIterator::~DirectoryIterator()
{
    if (IsValid())
    {
        System::Call(SYS_CLOSE, (USIZE)handle);
        handle = (PVOID)INVALID_FD;
    }
}

BOOL DirectoryIterator::Next()
{
    if (!IsValid())
        return FALSE;

    // If bpos >= nread, we need to ask the kernel for more entries
    if (first || bpos >= nread)
    {
        first = FALSE;
        nread = (INT32)System::Call(SYS_GETDENTS64, (USIZE)handle, (USIZE)buffer, sizeof(buffer));

        if (nread <= 0)
            return FALSE;  // End of directory or error
        bpos = 0;
    }

    linux_dirent64* d = (linux_dirent64*)(buffer + bpos);

    // Convert UTF-8 filename to wide string (UTF-16)
    String::Utf8ToWide(d->d_name, currentEntry.name, 256);

    // d_type 4 is directory
    currentEntry.isDirectory = (d->d_type == DT_DIR);
    currentEntry.isDrive = FALSE;  // Linux uses mount points, not drive letters
    currentEntry.type = (UINT32)d->d_type;
    currentEntry.isHidden = (d->d_name[0] == '.');
    currentEntry.isSystem = FALSE;
    currentEntry.isReadOnly = FALSE;
    currentEntry.size = 0;
    currentEntry.creationTime = 0;

    // Update buffer position for next call
    bpos += d->d_reclen;

    return TRUE;
}

BOOL DirectoryIterator::IsValid() const
{
    return (SSIZE)handle >= 0;
}
