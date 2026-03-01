#pragma once

#include "core/types/primitives.h"

struct EFI_CONTEXT;

// IA32_GS_BASE MSR number for x86_64
inline constexpr UINT32 IA32_GS_BASE = 0xC0000101;

/**
 * SetEfiContextRegister - Store context pointer in CPU register
 *
 * Uses GS base via MSR (IA32_GS_BASE = 0xC0000101).
 *
 * Note: We use WRMSR instead of WRGSBASE because WRGSBASE requires
 * CR4.FSGSBASE to be enabled, which UEFI firmware may not set.
 * WRMSR works in ring 0 (where UEFI apps run) without this requirement.
 *
 * @param ctx Reference to the EFI context to store
 */
inline VOID SetEfiContextRegister(EFI_CONTEXT &ctx)
{
	UINT64 value = reinterpret_cast<UINT64>(&ctx);
	UINT32 low = static_cast<UINT32>(value);
	UINT32 high = static_cast<UINT32>(value >> 32);
	__asm__ volatile(
		"wrmsr"
		:
		: "c"(IA32_GS_BASE), "a"(low), "d"(high)
		: "memory");
}

/**
 * GetEfiContext - Get the EFI context from CPU register
 *
 * Retrieves the context pointer from GS base via MSR (IA32_GS_BASE = 0xC0000101).
 *
 * @return Pointer to the EFI context
 */
inline EFI_CONTEXT *GetEfiContext()
{
	UINT32 low, high;
	__asm__ volatile(
		"rdmsr"
		: "=a"(low), "=d"(high)
		: "c"(IA32_GS_BASE));
	return reinterpret_cast<EFI_CONTEXT *>((static_cast<UINT64>(high) << 32) | low);
}
