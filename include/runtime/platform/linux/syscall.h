/**
 * syscall.h - Linux System Call Interface for Position-Independent Code
 *
 * This header provides a minimal, position-independent interface to Linux
 * kernel system calls. Instead of using libc wrappers (which would introduce
 * dependencies on .got/.plt sections), we invoke syscalls directly via
 * inline assembly.
 *
 * DESIGN PHILOSOPHY:
 *   - Zero libc dependencies - calls kernel directly
 *   - Position independent - no relocations needed
 *   - Minimal interface - only syscalls needed for the runtime
 *   - Multi-architecture - supports x86_64, i386, aarch64, armv7a
 *
 * SYSCALL MECHANISM:
 *   Linux syscalls are invoked differently on each architecture:
 *
 *   x86_64:  syscall instruction
 *            RAX=nr, RDI=a1, RSI=a2, RDX=a3, R10=a4, R8=a5, R9=a6
 *            Return in RAX, clobbers RCX and R11
 *
 *   i386:    int 0x80 instruction
 *            EAX=nr, EBX=a1, ECX=a2, EDX=a3, ESI=a4, EDI=a5, EBP=a6
 *            Return in EAX
 *
 *   aarch64: svc #0 instruction
 *            X8=nr, X0=a1, X1=a2, X2=a3, X3=a4, X4=a5, X5=a6
 *            Return in X0
 *
 *   armv7a:  svc 0 instruction (EABI)
 *            R7=nr, R0=a1, R1=a2, R2=a3, R3=a4, R4=a5, R5=a6
 *            Return in R0
 *
 * ERROR HANDLING:
 *   Linux syscalls return negative errno values on error (e.g., -ENOMEM).
 *   The caller must check for negative return values.
 *
 * USAGE:
 *   #include "linux/syscall.h"
 *   SSIZE bytes = Syscall::Write(STDOUT_FILENO, buffer, length);
 *   if (bytes < 0) { // handle error, -bytes is errno }
 */

#if defined(PLATFORM_LINUX)
#pragma once

#include "platform.h"

/* ============================================================================
 * POSIX/Linux Constants
 * ============================================================================
 * These constants match the Linux kernel ABI and are architecture-independent.
 * They are defined here to avoid any libc header dependencies.
 */

/* ----------------------------------------------------------------------------
 * Standard File Descriptors
 * ----------------------------------------------------------------------------
 * Every Unix process starts with three open file descriptors:
 *   0 (stdin)  - Standard input, typically the keyboard
 *   1 (stdout) - Standard output, typically the terminal
 *   2 (stderr) - Standard error, typically the terminal (unbuffered)
 *
 * These are guaranteed by POSIX and the Linux kernel.
 */
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* ----------------------------------------------------------------------------
 * Memory Protection Flags (for mmap)
 * ----------------------------------------------------------------------------
 * These flags control the access permissions of mapped memory pages.
 * They can be OR'd together (e.g., PROT_READ | PROT_WRITE).
 *
 * PROT_NONE  (0x00) - Page cannot be accessed (guard page)
 * PROT_READ  (0x01) - Page can be read
 * PROT_WRITE (0x02) - Page can be written
 * PROT_EXEC  (0x04) - Page can be executed (not used in this runtime)
 *
 * Note: We only define what we use to minimize the API surface.
 */
#define PROT_READ  0x01
#define PROT_WRITE 0x02

/* ----------------------------------------------------------------------------
 * Memory Mapping Flags (for mmap)
 * ----------------------------------------------------------------------------
 * These flags control how memory is mapped and shared.
 *
 * MAP_SHARED    (0x01) - Share mapping with other processes (not used)
 * MAP_PRIVATE   (0x02) - Create private copy-on-write mapping
 * MAP_ANONYMOUS (0x20) - Mapping is not backed by any file
 *
 * For heap allocation, we use MAP_PRIVATE | MAP_ANONYMOUS to get
 * private, zero-initialized memory pages from the kernel.
 */
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

/* ----------------------------------------------------------------------------
 * mmap Error Return Value
 * ----------------------------------------------------------------------------
 * On failure, mmap returns MAP_FAILED (which is (void*)-1, not NULL).
 * This is a POSIX requirement that differs from typical error handling.
 *
 * Always check: if (ptr == MAP_FAILED) { handle error }
 */
#define MAP_FAILED ((PVOID)(-1))

/* ============================================================================
 * Syscall Class - Static Interface to Linux System Calls
 * ============================================================================
 * All methods are static - no instance needed. This provides:
 *   1. No global state to initialize
 *   2. No vtable in .rdata (important for PIC)
 *   3. Direct function calls (no virtual dispatch overhead)
 *
 * Each method is a thin wrapper around the corresponding syscall,
 * with the architecture-specific implementation in platform.linux.*.cc
 */
class Syscall
{
public:
    /* ------------------------------------------------------------------------
     * Write - Write data to a file descriptor
     * ------------------------------------------------------------------------
     * Writes up to 'count' bytes from 'buf' to file descriptor 'fd'.
     *
     * @param fd    - File descriptor (STDOUT_FILENO for console output)
     * @param buf   - Pointer to data buffer
     * @param count - Number of bytes to write
     * @return Number of bytes written on success, negative errno on error
     *
     * Common errors:
     *   -EBADF  (9)  - fd is not a valid file descriptor
     *   -EFAULT (14) - buf is outside accessible address space
     *   -EINTR  (4)  - Call interrupted by signal (retry)
     *   -EIO    (5)  - I/O error
     *
     * Note: May write fewer bytes than requested (short write).
     * Caller should loop until all data is written or error occurs.
     *
     * Syscall numbers:
     *   x86_64:  1 (SYS_write)
     *   i386:    4 (SYS_write)
     *   aarch64: 64 (SYS_write)
     *   armv7a:  4 (SYS_write)
     */
    static SSIZE Write(INT32 fd, PCVOID buf, USIZE count);

    /* ------------------------------------------------------------------------
     * Exit - Terminate the calling process
     * ------------------------------------------------------------------------
     * Terminates the process immediately with the given exit status.
     * This function never returns.
     *
     * @param status - Exit status (0 = success, non-zero = error)
     * @return Never returns (but declared SSIZE for consistency)
     *
     * Note: This calls the exit syscall directly, not exit_group.
     * In a single-threaded program, they are equivalent.
     * For multi-threaded, exit_group would terminate all threads.
     *
     * Syscall numbers:
     *   x86_64:  60 (SYS_exit)
     *   i386:    1 (SYS_exit)
     *   aarch64: 93 (SYS_exit)
     *   armv7a:  1 (SYS_exit)
     */
    static SSIZE Exit(INT32 status);

    /* ------------------------------------------------------------------------
     * Mmap - Map memory pages
     * ------------------------------------------------------------------------
     * Creates a new mapping in the virtual address space of the calling process.
     * Used for memory allocation in our runtime (replaces malloc).
     *
     * @param addr   - Preferred address (NULL = let kernel choose)
     * @param length - Size of mapping in bytes (rounded up to page size)
     * @param prot   - Memory protection (PROT_READ | PROT_WRITE)
     * @param flags  - Mapping flags (MAP_PRIVATE | MAP_ANONYMOUS)
     * @param fd     - File descriptor (-1 for anonymous mappings)
     * @param offset - Offset in file (0 for anonymous mappings)
     * @return Pointer to mapped area, or MAP_FAILED on error
     *
     * Common errors:
     *   -ENOMEM (12) - Insufficient memory or address space
     *   -EINVAL (22) - Invalid arguments
     *
     * Usage for heap allocation:
     *   void* p = Syscall::Mmap(NULL, size, PROT_READ|PROT_WRITE,
     *                          MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
     *
     * NOTE:
     *   32-bit platforms (i386, armv7a) use mmap2 syscall where offset
     *   is in pages, not bytes. Our wrapper handles this transparently.
     *
     * Syscall numbers:
     *   x86_64:  9 (SYS_mmap)
     *   i386:    192 (SYS_mmap2) - offset in 4KB pages
     *   aarch64: 222 (SYS_mmap)
     *   armv7a:  192 (SYS_mmap2) - offset in 4KB pages
     */
    static PVOID Mmap(PVOID addr, USIZE length, INT32 prot, INT32 flags, INT32 fd, SSIZE offset);

    /* ------------------------------------------------------------------------
     * Munmap - Unmap memory pages
     * ------------------------------------------------------------------------
     * Deletes the mapping for the specified address range.
     * Used for memory deallocation in our runtime (replaces free).
     *
     * @param addr   - Starting address of mapping (must be page-aligned)
     * @param length - Length of mapping to unmap
     * @return 0 on success, negative errno on error
     *
     * Common errors:
     *   -EINVAL (22) - addr is not page-aligned or length is invalid
     *
     * Important: After munmap, accessing the unmapped region causes SIGSEGV.
     *
     * Syscall numbers:
     *   x86_64:  11 (SYS_munmap)
     *   i386:    91 (SYS_munmap)
     *   aarch64: 215 (SYS_munmap)
     *   armv7a:  91 (SYS_munmap)
     */
    static SSIZE Munmap(PVOID addr, USIZE length);
};

#endif // PLATFORM_LINUX
