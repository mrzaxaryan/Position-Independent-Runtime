/**
 * platform.linux.x86_64.cc - x86_64 Linux Syscall Implementation
 *
 * This file implements the low-level __syscall() function for the x86_64
 * (AMD64) architecture. It uses the `syscall` instruction introduced with
 * AMD64 to invoke Linux kernel system calls.
 *
 * x86_64 SYSCALL CALLING CONVENTION:
 * ==================================
 * The syscall instruction is the modern, fast method for invoking system
 * calls on x86_64 Linux. It's significantly faster than the legacy int 0x80
 * method because it doesn't require a full interrupt context switch.
 *
 * Register usage:
 *   INPUT:
 *     RAX - System call number
 *     RDI - 1st argument
 *     RSI - 2nd argument
 *     RDX - 3rd argument
 *     R10 - 4th argument (NOT RCX - see note below)
 *     R8  - 5th argument
 *     R9  - 6th argument
 *
 *   OUTPUT:
 *     RAX - Return value (or negative errno on error)
 *
 *   CLOBBERED:
 *     RCX - Holds return address after syscall
 *     R11 - Holds RFLAGS after syscall
 *
 * WHY R10 INSTEAD OF RCX FOR 4TH ARGUMENT:
 *   The syscall instruction uses RCX internally to save the return address
 *   (RIP) before jumping to the kernel. Therefore, we can't use RCX to pass
 *   the 4th argument. Linux uses R10 instead.
 *
 *   User-space ABI:  RDI, RSI, RDX, RCX, R8, R9
 *   Syscall ABI:     RDI, RSI, RDX, R10, R8, R9
 *
 * PERFORMANCE NOTES:
 *   - syscall is ~3x faster than int 0x80 on modern CPUs
 *   - VDSO can make some syscalls even faster (clock_gettime, etc.)
 *   - We don't use VDSO to maintain position independence
 *
 * REFERENCE:
 *   - AMD64 Architecture Programmer's Manual, Volume 2 (SYSCALL instruction)
 *   - Linux kernel: arch/x86/entry/entry_64.S
 */

#if defined(PLATFORM_LINUX_X86_64)

#include "platform.h"

/**
 * __syscall - Invoke a Linux system call on x86_64
 *
 * This is the low-level syscall function that all Syscall::* methods use.
 * It sets up the registers according to the x86_64 Linux syscall ABI and
 * executes the syscall instruction.
 *
 * @param nr - System call number (e.g., 1 for write, 60 for exit)
 * @param a1 - 1st argument (passed in RDI)
 * @param a2 - 2nd argument (passed in RSI)
 * @param a3 - 3rd argument (passed in RDX)
 * @param a4 - 4th argument (passed in R10)
 * @param a5 - 5th argument (passed in R8)
 * @param a6 - 6th argument (passed in R9)
 * @return Syscall return value in RAX (negative = -errno on error)
 *
 * INLINE ASSEMBLY EXPLANATION:
 *
 * The asm volatile statement has four parts:
 *   1. Template string: "syscall"
 *   2. Output operands: "=a"(ret) - RAX contains return value
 *   3. Input operands:  "a"(nr), "D"(a1), etc.
 *   4. Clobber list:    "rcx", "r11", "memory"
 *
 * Constraint letters:
 *   "a" = RAX register
 *   "D" = RDI register
 *   "S" = RSI register
 *   "d" = RDX register
 *   "r" = any general register (for r10, r8, r9 via register variables)
 *
 * Why register variables for R10, R8, R9:
 *   GCC inline asm doesn't have direct constraint letters for these registers.
 *   We use `register ... __asm__("r10")` to force the variable into that
 *   specific register, then use "r" constraint to tell GCC it's used.
 *
 * Why "volatile":
 *   Prevents the compiler from optimizing away or reordering the syscall.
 *   Syscalls have side effects (writing to files, allocating memory, etc.)
 *   that the compiler cannot see.
 *
 * Why "memory" clobber:
 *   Tells the compiler that the syscall may read or write arbitrary memory.
 *   This prevents the compiler from caching memory values across the syscall.
 */
SSIZE __syscall(SSIZE nr,
                USIZE a1, USIZE a2, USIZE a3,
                USIZE a4, USIZE a5, USIZE a6)
{
    SSIZE ret;

    /*
     * Load arguments 4-6 into R10, R8, R9 using register variables
     *
     * These register variables force the compiler to use specific registers.
     * The __asm__("r10") syntax is a GCC extension that binds the variable
     * to a specific register.
     */
    register SSIZE r10 __asm__("r10") = (SSIZE)a4;
    register SSIZE r8  __asm__("r8")  = (SSIZE)a5;
    register SSIZE r9  __asm__("r9")  = (SSIZE)a6;

    __asm__ volatile(
        "syscall"                /* Execute the syscall instruction */
        : "=a"(ret)              /* Output: RAX -> ret */
        : "a"(nr),               /* Input: nr -> RAX (syscall number) */
          "D"(a1),               /* Input: a1 -> RDI (1st argument) */
          "S"(a2),               /* Input: a2 -> RSI (2nd argument) */
          "d"(a3),               /* Input: a3 -> RDX (3rd argument) */
          "r"(r10),              /* Input: a4 -> R10 (4th argument) */
          "r"(r8),               /* Input: a5 -> R8  (5th argument) */
          "r"(r9)                /* Input: a6 -> R9  (6th argument) */
        : "rcx",                 /* Clobbered: RCX holds return address */
          "r11",                 /* Clobbered: R11 holds saved RFLAGS */
          "memory"               /* Clobbered: syscall may access memory */
    );

    return ret;
}

#endif /* PLATFORM_LINUX_X86_64 */
