/**
 * memory.h - Unified Memory Operations Interface
 *
 * Provides CRT-free memory manipulation functions (copy, set, compare, zero).
 * All operations are implemented using direct system calls or inline loops.
 *
 * REFACTORING NOTE:
 *   Previously had 3-level indirection: memcpy → Allocator::CopyMemory → Memory::Copy
 *   Now consolidated: Memory methods are thin inline wrappers over Allocator
 *   This eliminates unnecessary function call overhead while maintaining clean API
 *
 * DESIGN DECISION:
 *   Memory class provides the public API (short, intuitive names)
 *   Allocator class provides platform-specific implementation
 *   All Memory methods are FORCE_INLINE for zero overhead
 */

#pragma once

#include "allocator.h" // Platform-specific memory operations

/**
 * Memory - Position-independent memory operations
 *
 * All methods are static inline wrappers over Allocator.
 * No .cc file needed - zero function call overhead.
 *
 * USAGE:
 *   Memory::Copy(dest, src, size);   // memcpy equivalent
 *   Memory::Zero(buffer, size);      // memset(buffer, 0, size)
 *   Memory::Set(buffer, 'A', size);  // memset(buffer, 'A', size)
 *   Memory::Compare(a, b, size);     // memcmp equivalent
 */
class Memory
{
public:
	/**
	 * Copy - Copy memory block (memcpy equivalent)
	 *
	 * @param dest  - Destination buffer
	 * @param src   - Source buffer
	 * @param count - Number of bytes to copy
	 * @return dest pointer
	 *
	 * PERFORMANCE: Inlined to avoid call overhead on hot paths
	 * SAFETY: No overlap checking (undefined if src/dest overlap - use memmove for that)
	 */
	FORCE_INLINE static PVOID Copy(PVOID dest, PCVOID src, USIZE count)
	{
		return Allocator::CopyMemory(dest, src, count);
	}

	/**
	 * Zero - Fill memory with zeros (memset(ptr, 0, count) equivalent)
	 *
	 * @param dest  - Buffer to zero
	 * @param count - Number of bytes to zero
	 * @return dest pointer
	 *
	 * PERFORMANCE: Optimized for zeroing (common operation)
	 * USAGE: Buffer initialization, clearing sensitive data
	 */
	FORCE_INLINE static PVOID Zero(PVOID dest, USIZE count)
	{
		return Allocator::SetMemory(dest, 0, count);
	}

	/**
	 * Set - Fill memory with byte value (memset equivalent)
	 *
	 * @param dest  - Buffer to fill
	 * @param ch    - Byte value to write (0-255)
	 * @param count - Number of bytes to write
	 * @return dest pointer
	 *
	 * USAGE: Buffer initialization, pattern filling
	 */
	FORCE_INLINE static PVOID Set(PVOID dest, INT32 ch, USIZE count)
	{
		return Allocator::SetMemory(dest, ch, count);
	}

	/**
	 * Compare - Compare two memory blocks (memcmp equivalent)
	 *
	 * @param ptr1 - First buffer
	 * @param ptr2 - Second buffer
	 * @param num  - Number of bytes to compare
	 * @return <0 if ptr1 < ptr2, 0 if equal, >0 if ptr1 > ptr2
	 *
	 * PERFORMANCE: Byte-by-byte comparison (no SSE vectorization to avoid .rdata)
	 * USAGE: Buffer validation, hash verification, string comparison
	 */
	FORCE_INLINE static INT32 Compare(PCVOID ptr1, PCVOID ptr2, USIZE num)
	{
		return Allocator::CompareMemory(ptr1, ptr2, num);
	}
};
