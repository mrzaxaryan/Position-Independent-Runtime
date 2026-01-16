#if defined(PLATFORM_LINUX_X86_64)

#include "platform.h"

SSIZE __syscall(SSIZE nr,
				USIZE a1, USIZE a2, USIZE a3,
				USIZE a4, USIZE a5, USIZE a6)
{
	SSIZE ret;

	// r10, r8, r9 must be loaded explicitly before syscall
	register SSIZE r10 __asm__("r10") = (SSIZE)a4;
	register SSIZE r8 __asm__("r8") = (SSIZE)a5;
	register SSIZE r9 __asm__("r9") = (SSIZE)a6;

	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(nr),	// rax: syscall number
		  "D"(a1),	// rdi
		  "S"(a2),	// rsi
		  "d"(a3),	// rdx
		  "r"(r10), // r10
		  "r"(r8),	// r8
		  "r"(r9)	// r9
		: "rcx", "r11", "memory");

	return ret;
}

#endif // PLATFORM_LINUX_X86_64