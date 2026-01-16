#if defined(PLATFORM_LINUX_I386)

#include "platform.h"

// Generic 6-argument Linux i386 syscall helper
SSIZE __syscall(SSIZE nr,
				USIZE a1, USIZE a2, USIZE a3,
				USIZE a4, USIZE a5, USIZE a6)
{
	SSIZE ret;

	__asm__ volatile(
		"pushl %%ebx\n\t"
		"pushl %%esi\n\t"
		"pushl %%edi\n\t"
		"pushl %%ebp\n\t"
		"movl %1, %%eax\n\t" // nr
		"movl %2, %%ebx\n\t" // a1
		"movl %3, %%ecx\n\t" // a2
		"movl %4, %%edx\n\t" // a3
		"movl %5, %%esi\n\t" // a4
		"movl %6, %%edi\n\t" // a5
		"movl %7, %%ebp\n\t" // a6
		"int $0x80\n\t"
		"popl %%ebp\n\t"
		"popl %%edi\n\t"
		"popl %%esi\n\t"
		"popl %%ebx\n\t"
		: "=a"(ret)
		: "g"(nr), "g"(a1), "g"(a2), "g"(a3),
		  "g"(a4), "g"(a5), "g"(a6)
		: "memory", "cc", "ecx", "edx");
	return ret;
}

#endif