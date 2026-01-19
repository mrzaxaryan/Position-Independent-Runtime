/**
 * platform.linux.i386.cc - i386 Linux Syscall Implementation
 *
 * This file implements the low-level __syscall() function for the i386
 * (32-bit x86) architecture. It uses the `int 0x80` software interrupt
 * to invoke Linux kernel system calls.
 *
 * i386 SYSCALL CALLING CONVENTION:
 * ================================
 * The int 0x80 instruction triggers software interrupt 128, which the Linux
 * kernel handles as a system call request. This is the traditional method
 * for invoking syscalls on 32-bit x86 Linux.
 *
 * Register usage:
 *   INPUT:
 *     EAX - System call number
 *     EBX - 1st argument
 *     ECX - 2nd argument
 *     EDX - 3rd argument
 *     ESI - 4th argument
 *     EDI - 5th argument
 *     EBP - 6th argument
 *
 *   OUTPUT:
 *     EAX - Return value (or negative errno on error)
 *
 * REGISTER PRESERVATION:
 *   The kernel preserves all registers except EAX (return value).
 *   However, we explicitly save/restore EBX, ESI, EDI, EBP because
 *   they are callee-saved in the C ABI and our inline assembly
 *   modifies them.
 *
 * HISTORICAL NOTE:
 *   The int 0x80 mechanism dates back to the original Linux port to i386.
 *   While newer mechanisms exist (sysenter/syscall via VDSO), int 0x80
 *   remains supported for compatibility and works without VDSO, making
 *   it ideal for position-independent code.
 *
 * PERFORMANCE:
 *   int 0x80 is slower than sysenter/syscall because it goes through the
 *   full interrupt handling path. For our PIC runtime, correctness and
 *   position-independence are more important than syscall performance.
 *
 * REFERENCE:
 *   - Linux kernel: arch/x86/entry/entry_32.S
 *   - Intel SDM Volume 2: INT n instruction
 */

#if defined(PLATFORM_LINUX_I386)

#include "platform.h"

/**
 * __syscall - Invoke a Linux system call on i386
 *
 * This function sets up all six argument registers and invokes int 0x80.
 * Because we need to use EBX, ESI, EDI, and EBP (which are callee-saved
 * in the cdecl ABI), we must save and restore them around the syscall.
 *
 * @param nr - System call number (e.g., 4 for write, 1 for exit)
 * @param a1 - 1st argument (passed in EBX)
 * @param a2 - 2nd argument (passed in ECX)
 * @param a3 - 3rd argument (passed in EDX)
 * @param a4 - 4th argument (passed in ESI)
 * @param a5 - 5th argument (passed in EDI)
 * @param a6 - 6th argument (passed in EBP)
 * @return Syscall return value in EAX (negative = -errno on error)
 *
 * IMPLEMENTATION NOTES:
 *
 * We use a pure assembly implementation rather than register constraints
 * because:
 *   1. GCC's constraint system for i386 is complex with EBX (PIC register)
 *   2. We need to save/restore callee-saved registers explicitly
 *   3. EBP requires special handling (frame pointer)
 *
 * The push/pop sequence saves the callee-saved registers before we
 * overwrite them with syscall arguments, then restores them after.
 *
 * Register allocation:
 *   EAX - syscall number, then return value
 *   EBX - 1st argument (callee-saved, must save)
 *   ECX - 2nd argument (caller-saved, OK to clobber)
 *   EDX - 3rd argument (caller-saved, OK to clobber)
 *   ESI - 4th argument (callee-saved, must save)
 *   EDI - 5th argument (callee-saved, must save)
 *   EBP - 6th argument (callee-saved/frame pointer, must save)
 */
SSIZE __syscall(SSIZE nr,
                USIZE a1, USIZE a2, USIZE a3,
                USIZE a4, USIZE a5, USIZE a6)
{
    SSIZE ret;

    __asm__ volatile(
        /*
         * Save callee-saved registers
         *
         * In the cdecl ABI, EBX, ESI, EDI, and EBP are callee-saved.
         * We're about to overwrite them with syscall arguments, so we
         * must save them first and restore them after the syscall.
         */
        "pushl %%ebx\n\t"        /* Save EBX (also used for PIC) */
        "pushl %%esi\n\t"        /* Save ESI */
        "pushl %%edi\n\t"        /* Save EDI */
        "pushl %%ebp\n\t"        /* Save EBP (frame pointer) */

        /*
         * Load syscall arguments into registers
         *
         * We use movl with operand references (%1, %2, etc.) that
         * correspond to the input operands defined below.
         */
        "movl %1, %%eax\n\t"     /* EAX = syscall number (nr) */
        "movl %2, %%ebx\n\t"     /* EBX = 1st argument (a1) */
        "movl %3, %%ecx\n\t"     /* ECX = 2nd argument (a2) */
        "movl %4, %%edx\n\t"     /* EDX = 3rd argument (a3) */
        "movl %5, %%esi\n\t"     /* ESI = 4th argument (a4) */
        "movl %6, %%edi\n\t"     /* EDI = 5th argument (a5) */
        "movl %7, %%ebp\n\t"     /* EBP = 6th argument (a6) */

        /*
         * Execute the syscall
         *
         * int 0x80 triggers software interrupt 128, which transfers
         * control to the kernel's syscall handler. The kernel reads
         * the syscall number from EAX and arguments from the other
         * registers, performs the operation, and returns the result
         * in EAX.
         */
        "int $0x80\n\t"

        /*
         * Restore callee-saved registers
         *
         * Pop in reverse order to match the pushes above.
         * EAX now contains the syscall return value.
         */
        "popl %%ebp\n\t"         /* Restore EBP */
        "popl %%edi\n\t"         /* Restore EDI */
        "popl %%esi\n\t"         /* Restore ESI */
        "popl %%ebx\n\t"         /* Restore EBX */

        : "=a"(ret)              /* Output: EAX -> ret */
        : "g"(nr),               /* Input %1: syscall number */
          "g"(a1),               /* Input %2: 1st argument */
          "g"(a2),               /* Input %3: 2nd argument */
          "g"(a3),               /* Input %4: 3rd argument */
          "g"(a4),               /* Input %5: 4th argument */
          "g"(a5),               /* Input %6: 5th argument */
          "g"(a6)                /* Input %7: 6th argument */
        : "memory",              /* Clobbered: syscall may access memory */
          "cc",                  /* Clobbered: condition codes (EFLAGS) */
          "ecx",                 /* Clobbered: ECX (caller-saved anyway) */
          "edx"                  /* Clobbered: EDX (caller-saved anyway) */
    );

    return ret;
}

#endif /* PLATFORM_LINUX_I386 */
