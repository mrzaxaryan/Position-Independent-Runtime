/**
 * platform.linux.aarch64.cc - AArch64 Linux Syscall Implementation
 *
 * This file implements the low-level __syscall() function for the AArch64
 * (64-bit ARM) architecture. It uses the `svc #0` (supervisor call)
 * instruction to invoke Linux kernel system calls.
 *
 * AArch64 SYSCALL CALLING CONVENTION:
 * ===================================
 * AArch64 uses the svc (supervisor call) instruction to transition from
 * user mode (EL0) to kernel mode (EL1). The immediate operand (#0) is
 * conventionally 0 for Linux syscalls.
 *
 * Register usage:
 *   INPUT:
 *     X8  - System call number
 *     X0  - 1st argument
 *     X1  - 2nd argument
 *     X2  - 3rd argument
 *     X3  - 4th argument
 *     X4  - 5th argument
 *     X5  - 6th argument
 *
 *   OUTPUT:
 *     X0  - Return value (or negative errno on error)
 *
 *   PRESERVED:
 *     X1-X7, X9-X15 are preserved across the syscall
 *     X8 may be clobbered
 *
 * DESIGN NOTES:
 *   AArch64 has a clean, modern syscall ABI:
 *   - Syscall number in X8 (separate from arguments)
 *   - Arguments in X0-X5 (same as function call ABI for first 6 args)
 *   - Return value in X0 (same as function call ABI)
 *
 *   This similarity to the standard AAPCS64 calling convention makes
 *   syscall wrappers straightforward to implement.
 *
 * EXCEPTION LEVELS:
 *   - EL0: User mode (applications)
 *   - EL1: Kernel mode (Linux kernel)
 *   - EL2: Hypervisor mode
 *   - EL3: Secure monitor mode
 *
 *   svc causes a synchronous exception that transitions from EL0 to EL1.
 *
 * REFERENCE:
 *   - ARM Architecture Reference Manual for ARMv8-A
 *   - Linux kernel: arch/arm64/kernel/entry.S
 */

#if defined(PLATFORM_LINUX_AARCH64)

#include "platform.h"

/**
 * __syscall - Invoke a Linux system call on AArch64
 *
 * This function loads the syscall number into X8, arguments into X0-X5,
 * and executes svc #0 to invoke the kernel.
 *
 * @param nr - System call number (e.g., 64 for write, 93 for exit)
 * @param a1 - 1st argument (passed in X0)
 * @param a2 - 2nd argument (passed in X1)
 * @param a3 - 3rd argument (passed in X2)
 * @param a4 - 4th argument (passed in X3)
 * @param a5 - 5th argument (passed in X4)
 * @param a6 - 6th argument (passed in X5)
 * @return Syscall return value in X0 (negative = -errno on error)
 *
 * REGISTER VARIABLE APPROACH:
 *   We use register variables with explicit register bindings to ensure
 *   arguments are placed in the correct registers for the syscall.
 *
 *   register SSIZE x0 __asm__("x0") = (SSIZE)a1;
 *
 *   This tells the compiler to allocate 'x0' in the X0 register.
 *   Combined with the "r" constraint in the asm block, this guarantees
 *   the value is in the correct register when svc executes.
 *
 * WHY X0 IS BOTH INPUT AND OUTPUT:
 *   X0 serves double duty:
 *   - Before svc: holds 1st argument
 *   - After svc: holds return value
 *
 *   We use the "+r" constraint (read-write) for x0 to indicate this.
 *   The compiler knows X0's value changes across the svc instruction.
 */
SSIZE __syscall(SSIZE nr,
                USIZE a1, USIZE a2, USIZE a3,
                USIZE a4, USIZE a5, USIZE a6)
{
    /*
     * Bind arguments to specific registers
     *
     * AArch64 syscall ABI:
     *   X8 = syscall number
     *   X0 = 1st argument (also return value)
     *   X1 = 2nd argument
     *   X2 = 3rd argument
     *   X3 = 4th argument
     *   X4 = 5th argument
     *   X5 = 6th argument
     */
    register SSIZE x8 __asm__("x8") = nr;           /* Syscall number */
    register SSIZE x0 __asm__("x0") = (SSIZE)a1;    /* 1st arg / return */
    register SSIZE x1 __asm__("x1") = (SSIZE)a2;    /* 2nd argument */
    register SSIZE x2 __asm__("x2") = (SSIZE)a3;    /* 3rd argument */
    register SSIZE x3 __asm__("x3") = (SSIZE)a4;    /* 4th argument */
    register SSIZE x4 __asm__("x4") = (SSIZE)a5;    /* 5th argument */
    register SSIZE x5 __asm__("x5") = (SSIZE)a6;    /* 6th argument */

    __asm__ volatile(
        /*
         * svc #0 - Supervisor Call
         *
         * This instruction causes a synchronous exception that transfers
         * control to the kernel's syscall handler at EL1. The #0 immediate
         * is ignored by Linux but conventionally set to 0.
         *
         * The kernel:
         *   1. Saves user context
         *   2. Reads syscall number from X8
         *   3. Dispatches to appropriate handler
         *   4. Places return value in X0
         *   5. Returns to user mode via eret
         */
        "svc #0"

        : "+r"(x0)               /* Output: X0 is read-write (arg1 -> retval) */
        : "r"(x8),               /* Input: X8 = syscall number */
          "r"(x1),               /* Input: X1 = 2nd argument */
          "r"(x2),               /* Input: X2 = 3rd argument */
          "r"(x3),               /* Input: X3 = 4th argument */
          "r"(x4),               /* Input: X4 = 5th argument */
          "r"(x5)                /* Input: X5 = 6th argument */
        : "memory",              /* Clobbered: syscall may access memory */
          "cc"                   /* Clobbered: condition codes (NZCV flags) */
    );

    return x0;
}

#endif /* PLATFORM_LINUX_AARCH64 */
