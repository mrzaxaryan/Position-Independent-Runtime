#pragma once

#include "core/types/primitives.h"

// Solaris/illumos AArch64 syscall wrappers
// aarch64: svc #0 with X8 as the syscall number (standard ARM64
//          convention, NOT svc #0x80 / X16 like macOS). Carry flag (C bit in NZCV)
//          indicates error. We negate X0 on error to match the Linux convention.
class System
{
public:

	// Syscall with 0 arguments
	// Note: Solaris kernel may write rval[1] to X1 on return, so X1 must be clobbered.
	static inline SSIZE Call(USIZE number)
	{
		register USIZE x0 __asm__("x0");
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			"b.cc 1f\n"
			"neg x0, x0\n"
			"1:\n"
			: "=r"(x0)
			: "r"(x8)
			: "x1", "memory", "cc"
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
			"b.cc 1f\n"
			"neg x0, x0\n"
			"1:\n"
			: "+r"(x0)
			: "r"(x8)
			: "x1", "memory", "cc"
		);
		return (SSIZE)x0;
	}

	// Syscall with 2 arguments
	// X1 is used as input and clobbered by kernel (rval[1]), so mark as "+r".
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2)
	{
		register USIZE x0 __asm__("x0") = arg1;
		register USIZE x1 __asm__("x1") = arg2;
		register USIZE x8 __asm__("x8") = number;
		__asm__ volatile(
			"svc #0\n"
			"b.cc 1f\n"
			"neg x0, x0\n"
			"1:\n"
			: "+r"(x0), "+r"(x1)
			: "r"(x8)
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
			"b.cc 1f\n"
			"neg x0, x0\n"
			"1:\n"
			: "+r"(x0), "+r"(x1)
			: "r"(x2), "r"(x8)
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
			"b.cc 1f\n"
			"neg x0, x0\n"
			"1:\n"
			: "+r"(x0), "+r"(x1)
			: "r"(x2), "r"(x3), "r"(x8)
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
			"b.cc 1f\n"
			"neg x0, x0\n"
			"1:\n"
			: "+r"(x0), "+r"(x1)
			: "r"(x2), "r"(x3), "r"(x4), "r"(x8)
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
			"b.cc 1f\n"
			"neg x0, x0\n"
			"1:\n"
			: "+r"(x0), "+r"(x1)
			: "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8)
			: "memory", "cc"
		);
		return (SSIZE)x0;
	}

};  // class System
