#include "linux/syscall.h"

/* Variadic syscall macro - automatically pads unused arguments with 0 */
#define syscall1(n)                   __syscall(n, 0, 0, 0, 0, 0, 0)
#define syscall2(n, a)                __syscall(n, (USIZE)(a), 0, 0, 0, 0, 0)
#define syscall3(n, a, b)             __syscall(n, (USIZE)(a), (USIZE)(b), 0, 0, 0, 0)
#define syscall4(n, a, b, c)          __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), 0, 0, 0)
#define syscall5(n, a, b, c, d)       __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), (USIZE)(d), 0, 0)
#define syscall6(n, a, b, c, d, e)    __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), (USIZE)(d), (USIZE)(e), 0)
#define syscall7(n, a, b, c, d, e, f) __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), (USIZE)(d), (USIZE)(e), (USIZE)(f))

#define __SC_NARGS(_1,_2,_3,_4,_5,_6,_7,N,...) N
#define __SC_CAT(a, b) a##b
#define __SC_SELECT(N) __SC_CAT(syscall, N)
#define __syscall_raw(...) __SC_SELECT(__SC_NARGS(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1))(__VA_ARGS__)

/* -------------------------------------------------------------------------- */
/* Platform-specific syscall numbers                                          */
/* -------------------------------------------------------------------------- */
#if defined(PLATFORM_LINUX_X86_64)

/* File I/O */
#define SYS_write 1

/* Process operations */
#define SYS_exit 60

/* Memory operations */
#define SYS_mmap 9
#define SYS_munmap 11

/* -------------------------------------------------------------------------- */
/* PLATFORM_LINUX_AARCH64                                                      */
/* -------------------------------------------------------------------------- */
#elif defined(PLATFORM_LINUX_AARCH64)

/* File I/O */
#define SYS_write 64

/* Process operations */
#define SYS_exit 93

/* Memory operations */
#define SYS_mmap 222
#define SYS_munmap 215

/* -------------------------------------------------------------------------- */
/* PLATFORM_LINUX_I386 (x86 32-bit)                                            */
/* -------------------------------------------------------------------------- */
#elif defined(PLATFORM_LINUX_I386)

/* File I/O */
#define SYS_write 4

/* Process operations */
#define SYS_exit 1

/* Memory operations (i386 uses mmap2) */
#define SYS_mmap2 192
#define SYS_munmap 91

/* -------------------------------------------------------------------------- */
/* PLATFORM_LINUX_ARMV7A (ARM 32-bit)                                          */
/* -------------------------------------------------------------------------- */
#elif defined(PLATFORM_LINUX_ARMV7A)

/* File I/O */
#define SYS_write 4

/* Process operations */
#define SYS_exit 1

/* Memory operations (ARMV7A uses mmap2) */
#define SYS_mmap2 192
#define SYS_munmap 91

#endif

/* -------------------------------------------------------------------------- */
/* Syscall Wrapper Implementation                                             */
/* -------------------------------------------------------------------------- */

/* File I/O */
SSIZE Syscall::Write(INT32 fd, PCVOID buf, USIZE count)
{
    return __syscall_raw(SYS_write, fd, buf, count);
}

/* Process operations */
SSIZE Syscall::Exit(INT32 status)
{
    return __syscall_raw(SYS_exit, status);
}

/* Memory operations */
#if defined(PLATFORM_LINUX_I386) || defined(PLATFORM_LINUX_ARMV7A)
/* 32-bit platforms use mmap2 (offset in pages) */
PVOID Syscall::Mmap(PVOID addr, USIZE length, INT32 prot, INT32 flags, INT32 fd, SSIZE offset)
{
    return (PVOID)__syscall_raw(SYS_mmap2, addr, length, prot, flags, fd, offset);
}
#else
/* 64-bit platforms use mmap directly */
PVOID Syscall::Mmap(PVOID addr, USIZE length, INT32 prot, INT32 flags, INT32 fd, SSIZE offset)
{
    return (PVOID)__syscall_raw(SYS_mmap, addr, length, prot, flags, fd, offset);
}
#endif

SSIZE Syscall::Munmap(PVOID addr, USIZE length)
{
    return __syscall_raw(SYS_munmap, addr, length);
}