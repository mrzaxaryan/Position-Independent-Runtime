#pragma once

#include "core/types/primitives.h"

// macOS BSD x86_64 syscall wrappers
// x86_64: syscall instruction, same register convention as Linux, but carry flag
//         indicates error (RAX = positive errno). We negate on error to match
//         the Linux convention (negative return = error).
class System
{
public:

	// Syscall with 0 arguments
	// Note: macOS kernel writes rval[1] to RDX on return, so RDX must be clobbered.
	static inline SSIZE Call(USIZE number)
	{
		register USIZE r_rax __asm__("rax") = number;
		__asm__ volatile(
			"syscall\n"
			"jnc 1f\n"
			"negq %%rax\n"
			"1:\n"
			: "+r"(r_rax)
			:
			: "rcx", "rdx", "r11", "memory", "cc"
		);
		return (SSIZE)r_rax;
	}

	// Syscall with 1 argument
	static inline SSIZE Call(USIZE number, USIZE arg1)
	{
		register USIZE r_rdi __asm__("rdi") = arg1;
		register USIZE r_rax __asm__("rax") = number;
		__asm__ volatile(
			"syscall\n"
			"jnc 1f\n"
			"negq %%rax\n"
			"1:\n"
			: "+r"(r_rax)
			: "r"(r_rdi)
			: "rcx", "rdx", "r11", "memory", "cc"
		);
		return (SSIZE)r_rax;
	}

	// Syscall with 2 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2)
	{
		register USIZE r_rdi __asm__("rdi") = arg1;
		register USIZE r_rsi __asm__("rsi") = arg2;
		register USIZE r_rax __asm__("rax") = number;
		__asm__ volatile(
			"syscall\n"
			"jnc 1f\n"
			"negq %%rax\n"
			"1:\n"
			: "+r"(r_rax)
			: "r"(r_rdi), "r"(r_rsi)
			: "rcx", "rdx", "r11", "memory", "cc"
		);
		return (SSIZE)r_rax;
	}

	// Syscall with 3 arguments
	// RDX is used as input and clobbered by kernel (rval[1]), so mark as "+r".
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3)
	{
		register USIZE r_rdi __asm__("rdi") = arg1;
		register USIZE r_rsi __asm__("rsi") = arg2;
		register USIZE r_rdx __asm__("rdx") = arg3;
		register USIZE r_rax __asm__("rax") = number;
		__asm__ volatile(
			"syscall\n"
			"jnc 1f\n"
			"negq %%rax\n"
			"1:\n"
			: "+r"(r_rax), "+r"(r_rdx)
			: "r"(r_rdi), "r"(r_rsi)
			: "rcx", "r11", "memory", "cc"
		);
		return (SSIZE)r_rax;
	}

	// Syscall with 4 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3, USIZE arg4)
	{
		register USIZE r_rdi __asm__("rdi") = arg1;
		register USIZE r_rsi __asm__("rsi") = arg2;
		register USIZE r_rdx __asm__("rdx") = arg3;
		register USIZE r_r10 __asm__("r10") = arg4;
		register USIZE r_rax __asm__("rax") = number;
		__asm__ volatile(
			"syscall\n"
			"jnc 1f\n"
			"negq %%rax\n"
			"1:\n"
			: "+r"(r_rax), "+r"(r_rdx)
			: "r"(r_rdi), "r"(r_rsi), "r"(r_r10)
			: "rcx", "r11", "memory", "cc"
		);
		return (SSIZE)r_rax;
	}

	// Syscall with 5 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3, USIZE arg4, USIZE arg5)
	{
		register USIZE r_rdi __asm__("rdi") = arg1;
		register USIZE r_rsi __asm__("rsi") = arg2;
		register USIZE r_rdx __asm__("rdx") = arg3;
		register USIZE r_r10 __asm__("r10") = arg4;
		register USIZE r_r8 __asm__("r8") = arg5;
		register USIZE r_rax __asm__("rax") = number;
		__asm__ volatile(
			"syscall\n"
			"jnc 1f\n"
			"negq %%rax\n"
			"1:\n"
			: "+r"(r_rax), "+r"(r_rdx)
			: "r"(r_rdi), "r"(r_rsi), "r"(r_r10), "r"(r_r8)
			: "rcx", "r11", "memory", "cc"
		);
		return (SSIZE)r_rax;
	}

	// Syscall with 6 arguments
	static inline SSIZE Call(USIZE number, USIZE arg1, USIZE arg2, USIZE arg3, USIZE arg4, USIZE arg5, USIZE arg6)
	{
		register USIZE r_rdi __asm__("rdi") = arg1;
		register USIZE r_rsi __asm__("rsi") = arg2;
		register USIZE r_rdx __asm__("rdx") = arg3;
		register USIZE r_r10 __asm__("r10") = arg4;
		register USIZE r_r8 __asm__("r8") = arg5;
		register USIZE r_r9 __asm__("r9") = arg6;
		register USIZE r_rax __asm__("rax") = number;
		__asm__ volatile(
			"syscall\n"
			"jnc 1f\n"
			"negq %%rax\n"
			"1:\n"
			: "+r"(r_rax), "+r"(r_rdx)
			: "r"(r_rdi), "r"(r_rsi), "r"(r_r10), "r"(r_r8), "r"(r_r9)
			: "rcx", "r11", "memory", "cc"
		);
		return (SSIZE)r_rax;
	}

};  // class System
