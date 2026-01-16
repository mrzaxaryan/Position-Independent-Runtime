#include "platform.h"

#if defined(PLATFORM_LINUX_ARMV7A)

// Generic 6-argument Linux ARM EABI syscall helper
SSIZE __syscall(SSIZE nr,
				USIZE a1, USIZE a2, USIZE a3,
				USIZE a4, USIZE a5, USIZE a6)
{
	// Linux ARM EABI:
	// r7 = syscall number
	// r0–r6 = args
	// r0 = return value (or -errno)
	register long r0 __asm__("r0") = (long)a1;
	register long r1 __asm__("r1") = (long)a2;
	register long r2 __asm__("r2") = (long)a3;
	register long r3 __asm__("r3") = (long)a4;
	register long r4 __asm__("r4") = (long)a5;
	register long r5 __asm__("r5") = (long)a6;
	register long r7 __asm__("r7") = (long)nr;

	__asm__ volatile(
		"svc 0"	   // EABI-style syscall (svc preferred over swi)
		: "+r"(r0) // r0 is both input (arg1) and output (return)
		: "r"(r1), "r"(r2), "r"(r3),
		  "r"(r4), "r"(r5), "r"(r7)
		: "memory", "cc");

	return (SSIZE)r0;
}

#endif // PLATFORM_LINUX_ARMV7A
