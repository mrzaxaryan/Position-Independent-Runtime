/**
 * platform.linux.armv7a.cc - ARMv7-A Linux Syscall Implementation
 *
 * This file implements the low-level __syscall() function for the ARMv7-A
 * (32-bit ARM) architecture using the EABI (Embedded Application Binary
 * Interface). It uses the `svc 0` instruction to invoke Linux kernel
 * system calls.
 *
 * ARMv7-A EABI SYSCALL CALLING CONVENTION:
 * ========================================
 * Linux on ARM uses the EABI (Embedded ABI) for system calls, which differs
 * from the older OABI (Old ABI). EABI is required for ARMv7 and later.
 *
 * Register usage (EABI):
 *   INPUT:
 *     R7  - System call number
 *     R0  - 1st argument
 *     R1  - 2nd argument
 *     R2  - 3rd argument
 *     R3  - 4th argument
 *     R4  - 5th argument
 *     R5  - 6th argument
 *
 *   OUTPUT:
 *     R0  - Return value (or negative errno on error)
 *
 *   PRESERVED:
 *     R4-R11 are callee-saved in AAPCS (ARM standard calling convention)
 *
 * EABI vs OABI:
 *   OABI: syscall number encoded in svc instruction (svc #NR)
 *   EABI: syscall number in R7, svc operand is 0 (svc 0 or svc #0)
 *
 *   EABI advantages:
 *   - More than 256 syscalls possible (OABI limited by immediate field)
 *   - Cleaner separation of instruction from data
 *   - Required for Thumb-2 mode
 *
 * THUMB vs ARM MODE:
 *   ARMv7-A supports both ARM (32-bit instructions) and Thumb-2 (mixed
 *   16/32-bit instructions) modes. The svc instruction works in both modes.
 *   Our code compiles in ARM mode by default, but the syscall mechanism
 *   is the same in either mode.
 *
 * REFERENCE:
 *   - ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition
 *   - Linux kernel: arch/arm/kernel/entry-common.S
 *   - ARM EABI specification
 */

#if defined(PLATFORM_LINUX_ARMV7A)

#include "platform.h"

/**
 * __syscall - Invoke a Linux system call on ARMv7-A (EABI)
 *
 * This function loads the syscall number into R7, arguments into R0-R5,
 * and executes svc 0 to invoke the kernel.
 *
 * @param nr - System call number (e.g., 4 for write, 1 for exit)
 * @param a1 - 1st argument (passed in R0)
 * @param a2 - 2nd argument (passed in R1)
 * @param a3 - 3rd argument (passed in R2)
 * @param a4 - 4th argument (passed in R3)
 * @param a5 - 5th argument (passed in R4)
 * @param a6 - 6th argument (passed in R5)
 * @return Syscall return value in R0 (negative = -errno on error)
 *
 * REGISTER CONSIDERATIONS:
 *   R0-R3: Arguments 1-4, also used for return values and temporaries
 *          These are caller-saved in AAPCS.
 *
 *   R4-R5: Arguments 5-6 for syscalls
 *          These are callee-saved in AAPCS, so the kernel preserves them.
 *
 *   R7:    Syscall number in EABI
 *          Callee-saved, but we explicitly set it.
 *
 * WHY LONG INSTEAD OF SSIZE:
 *   On 32-bit ARM, 'long' is 32 bits, matching register size exactly.
 *   Using 'long' for register variables ensures no unexpected sign
 *   extension or truncation issues. SSIZE should also be 32-bit on
 *   this platform, but 'long' is idiomatic for ARM register operations.
 */
SSIZE __syscall(SSIZE nr,
                USIZE a1, USIZE a2, USIZE a3,
                USIZE a4, USIZE a5, USIZE a6)
{
    /*
     * Bind arguments to specific ARM registers
     *
     * ARM EABI syscall convention:
     *   R7 = syscall number
     *   R0 = 1st argument (also return value)
     *   R1 = 2nd argument
     *   R2 = 3rd argument
     *   R3 = 4th argument
     *   R4 = 5th argument (callee-saved)
     *   R5 = 6th argument (callee-saved)
     *
     * Using 'long' type for 32-bit register values.
     */
    register long r0 __asm__("r0") = (long)a1;    /* 1st arg / return */
    register long r1 __asm__("r1") = (long)a2;    /* 2nd argument */
    register long r2 __asm__("r2") = (long)a3;    /* 3rd argument */
    register long r3 __asm__("r3") = (long)a4;    /* 4th argument */
    register long r4 __asm__("r4") = (long)a5;    /* 5th argument */
    register long r5 __asm__("r5") = (long)a6;    /* 6th argument */
    register long r7 __asm__("r7") = (long)nr;    /* Syscall number */

    __asm__ volatile(
        /*
         * svc 0 - Supervisor Call (EABI style)
         *
         * This instruction causes a software interrupt (SWI) that transfers
         * control to the kernel's syscall vector. The kernel:
         *   1. Saves user registers to kernel stack
         *   2. Reads syscall number from R7
         *   3. Validates and dispatches to handler
         *   4. Places return value in R0
         *   5. Returns to user mode
         *
         * Note: "svc 0" and "svc #0" are equivalent in GNU assembler.
         * We use "svc 0" to match the style in the Linux kernel sources.
         */
        "svc 0"

        : "+r"(r0)               /* Output: R0 is read-write (arg1 -> retval) */
        : "r"(r1),               /* Input: R1 = 2nd argument */
          "r"(r2),               /* Input: R2 = 3rd argument */
          "r"(r3),               /* Input: R3 = 4th argument */
          "r"(r4),               /* Input: R4 = 5th argument */
          "r"(r5),               /* Input: R5 = 6th argument */
          "r"(r7)                /* Input: R7 = syscall number */
        : "memory",              /* Clobbered: syscall may access memory */
          "cc"                   /* Clobbered: condition codes (CPSR flags) */
    );

    return (SSIZE)r0;
}

#endif /* PLATFORM_LINUX_ARMV7A */
