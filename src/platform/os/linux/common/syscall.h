#pragma once

#include "core/types/primitives.h"

// =============================================================================
// Linux Syscall Numbers (Architecture-specific)
// =============================================================================

#if defined(ARCHITECTURE_X86_64)
#include "platform/os/linux/common/syscall.x86_64.h"
#elif defined(ARCHITECTURE_I386)
#include "platform/os/linux/common/syscall.i386.h"
#elif defined(ARCHITECTURE_AARCH64)
#include "platform/os/linux/common/syscall.aarch64.h"
#elif defined(ARCHITECTURE_ARMV7A)
#include "platform/os/linux/common/syscall.armv7a.h"
#else
#error "Unsupported architecture"
#endif

// =============================================================================
// POSIX/Linux Constants
// =============================================================================

// Standard file descriptors
constexpr INT32 STDIN_FILENO = 0;
constexpr INT32 STDOUT_FILENO = 1;
constexpr INT32 STDERR_FILENO = 2;

// File open flags
constexpr INT32 O_RDONLY = 0x0000;
constexpr INT32 O_WRONLY = 0x0001;
constexpr INT32 O_RDWR = 0x0002;
constexpr INT32 O_CREAT = 0x0040;
constexpr INT32 O_TRUNC = 0x0200;
constexpr INT32 O_APPEND = 0x0400;
constexpr INT32 O_NONBLOCK = 0x0800;

#if defined(ARCHITECTURE_X86_64) || defined(ARCHITECTURE_I386)
constexpr INT32 O_DIRECTORY = 0x10000;
#elif defined(ARCHITECTURE_AARCH64) || defined(ARCHITECTURE_ARMV7A)
constexpr INT32 O_DIRECTORY = 0x4000;
#endif

// lseek whence values
constexpr INT32 SEEK_SET = 0;
constexpr INT32 SEEK_CUR = 1;
constexpr INT32 SEEK_END = 2;

// File mode/permission bits
constexpr INT32 S_IRUSR = 0x0100;  // User read
constexpr INT32 S_IWUSR = 0x0080;  // User write
constexpr INT32 S_IXUSR = 0x0040;  // User execute
constexpr INT32 S_IRGRP = 0x0020;  // Group read
constexpr INT32 S_IWGRP = 0x0010;  // Group write
constexpr INT32 S_IXGRP = 0x0008;  // Group execute
constexpr INT32 S_IROTH = 0x0004;  // Others read
constexpr INT32 S_IWOTH = 0x0002;  // Others write
constexpr INT32 S_IXOTH = 0x0001;  // Others execute

// Directory entry types
constexpr UINT8 DT_UNKNOWN = 0;
constexpr UINT8 DT_FIFO = 1;
constexpr UINT8 DT_CHR = 2;
constexpr UINT8 DT_DIR = 4;
constexpr UINT8 DT_BLK = 6;
constexpr UINT8 DT_REG = 8;
constexpr UINT8 DT_LNK = 10;
constexpr UINT8 DT_SOCK = 12;

// Memory protection flags
constexpr INT32 PROT_READ = 0x01;
constexpr INT32 PROT_WRITE = 0x02;
constexpr INT32 PROT_EXEC = 0x04;

// Memory mapping flags
constexpr INT32 MAP_PRIVATE = 0x02;
constexpr INT32 MAP_ANONYMOUS = 0x20;
#define MAP_FAILED ((PVOID)(-1))

// Clock IDs
constexpr INT32 CLOCK_REALTIME = 0;
constexpr INT32 CLOCK_MONOTONIC = 1;

// Socket options
constexpr INT32 SOL_SOCKET = 1;
constexpr INT32 SO_ERROR = 4;
constexpr INT32 SO_RCVTIMEO = 20;
constexpr INT32 SO_SNDTIMEO = 21;
constexpr INT32 IPPROTO_TCP = 6;
constexpr INT32 TCP_NODELAY = 1;

// fcntl commands
constexpr INT32 F_GETFL = 3;
constexpr INT32 F_SETFL = 4;

// errno values
constexpr INT32 EINPROGRESS = 115;

// poll event flags
constexpr INT16 POLLOUT = 0x0004;
constexpr INT16 POLLERR = 0x0008;
constexpr INT16 POLLHUP = 0x0010;

// Invalid file descriptor
constexpr SSIZE INVALID_FD = -1;

// =============================================================================
// Linux Structures
// =============================================================================

// Linux dirent64 structure for directory iteration
struct linux_dirent64
{
	UINT64 d_ino;
	INT64 d_off;
	UINT16 d_reclen;
	UINT8 d_type;
	CHAR d_name[];
};

// Timespec structure
struct timespec
{
	SSIZE tv_sec;
	SSIZE tv_nsec;
};

// Timeval structure (for socket timeouts)
struct timeval
{
	SSIZE tv_sec;
	SSIZE tv_usec;
};

// pollfd structure (for poll/ppoll)
struct pollfd
{
	INT32 fd;
	INT16 events;
	INT16 revents;
};
