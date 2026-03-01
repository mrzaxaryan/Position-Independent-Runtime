/**
 * efi_context.h - EFI Runtime Context
 *
 * Provides storage and access to EFI runtime context.
 * The ImageHandle and SystemTable are stored here after entry_point
 * receives them and can be accessed by all PLATFORM functions.
 */

#pragma once

#include "platform/os/uefi/common/efi_system_table.h"

// =============================================================================
// EFI Context Structure
// =============================================================================

struct EFI_CONTEXT
{
	EFI_HANDLE ImageHandle;
	EFI_SYSTEM_TABLE *SystemTable;
	BOOL NetworkInitialized;
	BOOL DhcpConfigured;
	BOOL TcpStackReady;
};

// =============================================================================
// Context Register Access — architecture-specific implementations
// =============================================================================

#if defined(ARCHITECTURE_X86_64)
#include "platform/os/uefi/common/efi_context.x86_64.h"
#elif defined(ARCHITECTURE_AARCH64)
#include "platform/os/uefi/common/efi_context.aarch64.h"
#else
#error "Unsupported architecture"
#endif
