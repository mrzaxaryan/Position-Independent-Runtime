/**
 * syscall.cc - Linux System Call Wrapper Implementations
 *
 * This file implements the Syscall class methods declared in syscall.h.
 * Each method is a thin wrapper that invokes the low-level __syscall()
 * function with the appropriate syscall number for the current architecture.
 *
 * ARCHITECTURE ABSTRACTION:
 *   The actual syscall invocation is architecture-specific and implemented in:
 *     - platform.linux.x86_64.cc  (syscall instruction)
 *     - platform.linux.i386.cc    (int 0x80)
 *     - platform.linux.aarch64.cc (svc #0)
 *     - platform.linux.armv7a.cc  (svc 0)
 *
 *   This file provides the architecture-independent wrapper layer that:
 *     1. Defines syscall numbers per architecture
 *     2. Calls __syscall() with correct arguments
 *     3. Handles 32-bit vs 64-bit differences (e.g., mmap vs mmap2)
 *
 * SYSCALL NUMBER SOURCES:
 *   x86_64:  /usr/include/asm/unistd_64.h
 *   i386:    /usr/include/asm/unistd_32.h
 *   aarch64: /usr/include/asm-generic/unistd.h
 *   armv7a:  /usr/include/arm-linux-gnueabi/asm/unistd.h
 *
 * VARIADIC SYSCALL MACROS:
 *   The syscall macros (syscall1, syscall2, etc.) automatically pad
 *   unused arguments with 0, simplifying the wrapper implementations.
 */

#include "linux/syscall.h"

/* ============================================================================
 * Variadic Syscall Helper Macros
 * ============================================================================
 * These macros provide a convenient way to invoke __syscall() with 1-7
 * arguments, automatically padding unused arguments with 0.
 *
 * The __syscall() function always takes 7 arguments (nr + 6 args), but most
 * syscalls need fewer. These macros handle the padding automatically.
 *
 * Example:
 *   syscall2(SYS_exit, 0)
 *   expands to:
 *   __syscall(SYS_exit, 0, 0, 0, 0, 0, 0)
 */

/* Direct invocation macros - pad unused args with 0 */
#define syscall1(n)                   __syscall(n, 0, 0, 0, 0, 0, 0)
#define syscall2(n, a)                __syscall(n, (USIZE)(a), 0, 0, 0, 0, 0)
#define syscall3(n, a, b)             __syscall(n, (USIZE)(a), (USIZE)(b), 0, 0, 0, 0)
#define syscall4(n, a, b, c)          __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), 0, 0, 0)
#define syscall5(n, a, b, c, d)       __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), (USIZE)(d), 0, 0)
#define syscall6(n, a, b, c, d, e)    __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), (USIZE)(d), (USIZE)(e), 0)
#define syscall7(n, a, b, c, d, e, f) __syscall(n, (USIZE)(a), (USIZE)(b), (USIZE)(c), (USIZE)(d), (USIZE)(e), (USIZE)(f))

/*
 * Argument counting magic for __syscall_raw() macro
 *
 * __SC_NARGS counts the number of arguments passed (1-7)
 * __SC_SELECT selects the appropriate syscallN macro
 * __syscall_raw dispatches to the correct macro based on arg count
 *
 * Example:
 *   __syscall_raw(SYS_write, fd, buf, count)
 *   -> __SC_NARGS(SYS_write, fd, buf, count, 7, 6, 5, 4, 3, 2, 1) = 4
 *   -> __SC_SELECT(4) = syscall4
 *   -> syscall4(SYS_write, fd, buf, count)
 */
#define __SC_NARGS(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define __SC_CAT(a, b) a##b
#define __SC_SELECT(N) __SC_CAT(syscall, N)
#define __syscall_raw(...) __SC_SELECT(__SC_NARGS(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1))(__VA_ARGS__)

/* ============================================================================
 * Platform-Specific Syscall Numbers
 * ============================================================================
 * Each architecture has its own set of syscall numbers. These are defined
 * by the Linux kernel ABI and are stable across kernel versions.
 *
 * Note: Some syscalls have different numbers on different architectures,
 * and some architectures have different syscalls for the same operation
 * (e.g., mmap vs mmap2 on 32-bit platforms).
 */

/* ----------------------------------------------------------------------------
 * x86_64 (64-bit x86) Syscall Numbers
 * ----------------------------------------------------------------------------
 * Source: arch/x86/entry/syscalls/syscall_64.tbl
 *
 * x86_64 uses the modern syscall instruction with:
 *   RAX = syscall number
 *   RDI, RSI, RDX, R10, R8, R9 = arguments 1-6
 *   RAX = return value
 */
#if defined(PLATFORM_LINUX_X86_64)

#define SYS_write  1   /* ssize_t write(int fd, const void *buf, size_t count) */
#define SYS_mmap   9   /* void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off) */
#define SYS_munmap 11  /* int munmap(void *addr, size_t length) */
#define SYS_exit   60  /* void exit(int status) - never returns */

/* ----------------------------------------------------------------------------
 * AArch64 (64-bit ARM) Syscall Numbers
 * ----------------------------------------------------------------------------
 * Source: include/uapi/asm-generic/unistd.h
 *
 * AArch64 uses the svc #0 instruction with:
 *   X8 = syscall number
 *   X0-X5 = arguments 1-6
 *   X0 = return value
 */
#elif defined(PLATFORM_LINUX_AARCH64)

#define SYS_write  64  /* ssize_t write(int fd, const void *buf, size_t count) */
#define SYS_mmap   222 /* void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off) */
#define SYS_munmap 215 /* int munmap(void *addr, size_t length) */
#define SYS_exit   93  /* void exit(int status) - never returns */

/* ----------------------------------------------------------------------------
 * i386 (32-bit x86) Syscall Numbers
 * ----------------------------------------------------------------------------
 * Source: arch/x86/entry/syscalls/syscall_32.tbl
 *
 * i386 uses the int 0x80 software interrupt with:
 *   EAX = syscall number
 *   EBX, ECX, EDX, ESI, EDI, EBP = arguments 1-6
 *   EAX = return value
 *
 * IMPORTANT: i386 uses mmap2 instead of mmap!
 *   mmap2 takes the offset in 4KB pages, not bytes.
 *   This allows mapping files larger than 4GB on 32-bit systems.
 */
#elif defined(PLATFORM_LINUX_I386)

#define SYS_write  4   /* ssize_t write(int fd, const void *buf, size_t count) */
#define SYS_mmap2  192 /* void *mmap2(void *addr, size_t len, int prot, int flags, int fd, off_t pgoff) */
#define SYS_munmap 91  /* int munmap(void *addr, size_t length) */
#define SYS_exit   1   /* void exit(int status) - never returns */

/* ----------------------------------------------------------------------------
 * ARMv7-A (32-bit ARM EABI) Syscall Numbers
 * ----------------------------------------------------------------------------
 * Source: arch/arm/tools/syscall.tbl
 *
 * ARMv7-A uses the svc 0 (supervisor call) instruction with EABI:
 *   R7 = syscall number
 *   R0-R5 = arguments 1-6
 *   R0 = return value
 *
 * IMPORTANT: Like i386, ARMv7-A uses mmap2 with offset in pages.
 */
#elif defined(PLATFORM_LINUX_ARMV7A)

#define SYS_write  4   /* ssize_t write(int fd, const void *buf, size_t count) */
#define SYS_mmap2  192 /* void *mmap2(void *addr, size_t len, int prot, int flags, int fd, off_t pgoff) */
#define SYS_munmap 91  /* int munmap(void *addr, size_t length) */
#define SYS_exit   1   /* void exit(int status) - never returns */

#endif

/* ============================================================================
 * Syscall Wrapper Implementations
 * ============================================================================
 * These methods implement the Syscall class interface defined in syscall.h.
 * Each method invokes __syscall_raw() with the appropriate syscall number
 * and arguments for the current architecture.
 */

/* ----------------------------------------------------------------------------
 * Syscall::Write - Write data to file descriptor
 * ----------------------------------------------------------------------------
 * Wrapper for the write(2) syscall.
 *
 * @param fd    - File descriptor to write to
 * @param buf   - Buffer containing data to write
 * @param count - Number of bytes to write
 * @return Bytes written on success, negative errno on error
 */
SSIZE Syscall::Write(INT32 fd, PCVOID buf, USIZE count)
{
    return __syscall_raw(SYS_write, fd, buf, count);
}

/* ----------------------------------------------------------------------------
 * Syscall::Exit - Terminate the process
 * ----------------------------------------------------------------------------
 * Wrapper for the exit(2) syscall. This function never returns.
 *
 * @param status - Exit status code (0 = success)
 * @return Never returns (declared SSIZE for interface consistency)
 */
SSIZE Syscall::Exit(INT32 status)
{
    return __syscall_raw(SYS_exit, status);
}

/* ----------------------------------------------------------------------------
 * Syscall::Mmap - Map memory pages
 * ----------------------------------------------------------------------------
 * Wrapper for mmap(2) / mmap2(2) syscall.
 *
 * On 32-bit platforms (i386, armv7a), we use mmap2 which expects the offset
 * in 4KB pages rather than bytes. Since we only use anonymous mappings
 * (offset = 0), this difference doesn't affect our usage.
 *
 * @param addr   - Preferred mapping address (NULL = kernel chooses)
 * @param length - Size of mapping in bytes
 * @param prot   - Protection flags (PROT_READ | PROT_WRITE)
 * @param flags  - Mapping flags (MAP_PRIVATE | MAP_ANONYMOUS)
 * @param fd     - File descriptor (-1 for anonymous)
 * @param offset - File offset (0 for anonymous)
 * @return Pointer to mapped memory, or MAP_FAILED on error
 */
#if defined(PLATFORM_LINUX_I386) || defined(PLATFORM_LINUX_ARMV7A)
/*
 * 32-bit platforms use mmap2 syscall
 *
 * mmap2 differs from mmap in that the offset is specified in 4KB page units
 * rather than bytes. For anonymous mappings (fd=-1, offset=0), this is
 * transparent to the caller.
 *
 * If we ever need file-backed mappings, we'd need to convert:
 *   mmap2_offset = byte_offset / 4096
 */
PVOID Syscall::Mmap(PVOID addr, USIZE length, INT32 prot, INT32 flags, INT32 fd, SSIZE offset)
{
    /* Note: offset is passed directly - for anonymous mappings it's always 0 */
    /* If file mappings were needed, offset would need to be divided by 4096 */
    return (PVOID)__syscall_raw(SYS_mmap2, addr, length, prot, flags, fd, offset);
}
#else
/*
 * 64-bit platforms use standard mmap syscall
 *
 * The offset is in bytes, which is more intuitive but limits file mappings
 * to files smaller than the address space (not a problem on 64-bit).
 */
PVOID Syscall::Mmap(PVOID addr, USIZE length, INT32 prot, INT32 flags, INT32 fd, SSIZE offset)
{
    return (PVOID)__syscall_raw(SYS_mmap, addr, length, prot, flags, fd, offset);
}
#endif

/* ----------------------------------------------------------------------------
 * Syscall::Munmap - Unmap memory pages
 * ----------------------------------------------------------------------------
 * Wrapper for the munmap(2) syscall.
 *
 * @param addr   - Starting address of mapping to unmap (must be page-aligned)
 * @param length - Length of region to unmap
 * @return 0 on success, negative errno on error
 */
SSIZE Syscall::Munmap(PVOID addr, USIZE length)
{
    return __syscall_raw(SYS_munmap, addr, length);
}
