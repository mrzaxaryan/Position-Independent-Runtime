/**
 * platform.h - Platform Abstraction Layer
 *
 * OS/hardware abstraction.
 * Depends on CORE.
 */

#pragma once

#include "core/core.h"

// =============================================================================
// Platform-Specific Headers
// =============================================================================

#if defined(PLATFORM_UEFI)
// UEFI platform - include EFI types and system table
#include "platform/os/uefi/common/efi_types.h"
#include "platform/os/uefi/common/efi_system_table.h"
#include "platform/os/uefi/common/efi_context.h"
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
#include "platform/memory/allocator.h"

// System utilities (must come before logger.h since it uses DateTime)
#include "platform/system/date_time.h"
#include "platform/system/random.h"

// I/O services
#include "platform/io/console.h"
#include "platform/io/file_system/offset_origin.h"
#include "platform/io/file_system/directory_entry.h"
#include "platform/io/file_system/file.h"
#include "platform/io/file_system/directory.h"
#include "platform/io/file_system/directory_iterator.h"
#include "platform/io/file_system/path.h"
#include "platform/io/logger.h"

// Network services
#include "platform/network/socket.h"

// Process management
#include "platform/system/process.h"
