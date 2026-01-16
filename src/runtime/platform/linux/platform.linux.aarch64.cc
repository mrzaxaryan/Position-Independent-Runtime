
#if defined(PLATFORM_LINUX_AARCH64)

#include "platform.h"

// Generic 6-argument Linux AArch64 syscall helper
// x8  = syscall number
// x0–x5 = up to 6 arguments
// x0  = return value (or -errno)
SSIZE __syscall(SSIZE nr,
				USIZE a1, USIZE a2, USIZE a3,
				USIZE a4, USIZE a5, USIZE a6)
{
	register SSIZE x8 __asm__("x8") = nr;
	register SSIZE x0 __asm__("x0") = (SSIZE)a1;
	register SSIZE x1 __asm__("x1") = (SSIZE)a2;
	register SSIZE x2 __asm__("x2") = (SSIZE)a3;
	register SSIZE x3 __asm__("x3") = (SSIZE)a4;
	register SSIZE x4 __asm__("x4") = (SSIZE)a5;
	register SSIZE x5 __asm__("x5") = (SSIZE)a6;

	__asm__ volatile(
		"svc #0"
		: "+r"(x0)
		: "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5)
		: "memory", "cc");

	return x0;
}

#endif // PLATFORM_LINUX_AARCH64
