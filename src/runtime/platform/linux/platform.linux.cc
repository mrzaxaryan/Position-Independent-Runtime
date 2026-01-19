/**
 * platform.linux.cc - Linux Platform Core Functions
 *
 * This file implements platform-specific functions that are common across
 * all Linux architectures (x86_64, i386, aarch64, armv7a).
 *
 * Currently implements:
 *   - ExitProcess() - Cross-platform process termination
 *
 * DESIGN NOTES:
 *   Unlike Windows which requires PEB walking and ntdll resolution,
 *   Linux process termination is straightforward - just invoke the
 *   exit syscall directly.
 *
 *   Architecture-specific syscall implementations are in separate files:
 *     - platform.linux.x86_64.cc
 *     - platform.linux.i386.cc
 *     - platform.linux.aarch64.cc
 *     - platform.linux.armv7a.cc
 */

#if defined(PLATFORM_LINUX)

#include "linux/syscall.h"

/**
 * ExitProcess - Terminate the current process
 *
 * This function terminates the process immediately with the given exit code.
 * It is the Linux equivalent of Windows' ExitProcess() / NtTerminateProcess().
 *
 * @param code - Exit status code
 *               0 = success (EXIT_SUCCESS)
 *               1-255 = error (EXIT_FAILURE is typically 1)
 *
 * IMPLEMENTATION:
 *   Calls the exit(2) syscall directly via Syscall::Exit().
 *   The syscall number varies by architecture:
 *     x86_64:  60
 *     i386:    1
 *     aarch64: 93
 *     armv7a:  1
 *
 * IMPORTANT:
 *   This function NEVER returns. The __builtin_unreachable() hint tells
 *   the compiler this, enabling better optimization and preventing
 *   "function may return without value" warnings.
 *
 * NOTE ON exit vs exit_group:
 *   - exit (SYS_exit) terminates only the calling thread
 *   - exit_group (SYS_exit_group) terminates all threads in the process
 *
 *   For single-threaded programs (which our PIC runtime is), these are
 *   equivalent. We use exit for simplicity and broader compatibility.
 *
 * CROSS-PLATFORM USAGE:
 *   This function provides a unified interface across Windows and Linux:
 *
 *   Windows: ExitProcess() -> NtTerminateProcess() -> kernel
 *   Linux:   ExitProcess() -> Syscall::Exit() -> kernel
 *
 *   Code using ExitProcess() works on both platforms without modification.
 */
NO_RETURN VOID ExitProcess(USIZE code)
{
    /* Invoke the exit syscall - this never returns */
    Syscall::Exit((INT32)code);

    /*
     * __builtin_unreachable() - Compiler hint for unreachable code
     *
     * This tells the compiler that execution will never reach this point.
     * Benefits:
     *   1. Suppresses "control reaches end of non-void function" warnings
     *   2. Enables dead code elimination after this call
     *   3. Allows the compiler to assume no return, optimizing callers
     *
     * If execution somehow did reach here (e.g., syscall failed to terminate),
     * the behavior is undefined - but that would indicate a kernel bug.
     */
    __builtin_unreachable();
}

#endif /* PLATFORM_LINUX */
