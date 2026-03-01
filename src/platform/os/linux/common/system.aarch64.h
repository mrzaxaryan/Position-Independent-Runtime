#pragma once

#include "core/types/primitives.h"

// Linux AArch64 syscall wrappers
class System
{
public:

	// Syscall with 0 arguments
	static inline SSIZE Call(USIZE number)
	{
		register USIZE x0 __asm__("x0");
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			: "=r"(x0)
			: "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

	// Syscall with 1 argument
	static inline SSIZE Call(USIZE number, USIZE arg1)
	{
		register USIZE x0 __asm__("x0") = arg1;
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			: "+r"(x0)
			: "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

	// Syscall with 2 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2)
	{
		register USIZE x0 __asm__("x0") = arg1;
		register USIZE x1 __asm__("x1") = arg2;
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			: "+r"(x0)
			: "r"(x1), "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

	// Syscall with 3 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3)
	{
		register USIZE x0 __asm__("x0") = arg1;
		register USIZE x1 __asm__("x1") = arg2;
		register USIZE x2 __asm__("x2") = arg3;
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			: "+r"(x0)
			: "r"(x1), "r"(x2), "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

	// Syscall with 4 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3, USIZE arg4)
	{
		register USIZE x0 __asm__("x0") = arg1;
		register USIZE x1 __asm__("x1") = arg2;
		register USIZE x2 __asm__("x2") = arg3;
		register USIZE x3 __asm__("x3") = arg4;
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			: "+r"(x0)
			: "r"(x1), "r"(x2), "r"(x3), "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

	// Syscall with 5 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3, USIZE arg4, USIZE arg5)
	{
		register USIZE x0 __asm__("x0") = arg1;
		register USIZE x1 __asm__("x1") = arg2;
		register USIZE x2 __asm__("x2") = arg3;
		register USIZE x3 __asm__("x3") = arg4;
		register USIZE x4 __asm__("x4") = arg5;
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			: "+r"(x0)
			: "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

	// Syscall with 6 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3, USIZE arg4, USIZE arg5, USIZE arg6)
	{
		register USIZE x0 __asm__("x0") = arg1;
		register USIZE x1 __asm__("x1") = arg2;
		register USIZE x2 __asm__("x2") = arg3;
		register USIZE x3 __asm__("x3") = arg4;
		register USIZE x4 __asm__("x4") = arg5;
		register USIZE x5 __asm__("x5") = arg6;
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			: "+r"(x0)
			: "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

};  // class System
