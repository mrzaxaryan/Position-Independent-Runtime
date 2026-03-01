/**
 * platform.h - Platform Abstraction Layer
 *
 * OS/hardware abstraction.
 * Depends on CORE.
 */

#pragma once

#include "core.h"

// =============================================================================
// Platform-Specific Headers
// =============================================================================

#if defined(PLATFORM_UEFI)
// UEFI platform - include EFI types and system table
#include "efi_types.h"
#include "efi_system_table.h"
#include "efi_context.h"
#endif

// =============================================================================
// Platform Core
// =============================================================================

// Cross-platform exit process function
NO_RETURN VOID ExitProcess(USIZE code);

// =============================================================================
// Platform Services
// =============================================================================

// Memory management
#include "allocator.h"

// System utilities (must come before logger.h since it uses DateTime)
#include "date_time.h"
#include "random.h"

// I/O services
#include "console.h"
#include "file_system.h"
#include "logger.h"

// Network services
#include "socket.h"
#include "path.h"

// Process management
#include "process.h"
