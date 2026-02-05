#pragma once

#include "primitives.h"
#include "compiler.h"

// Bit rotation operations for cryptographic algorithms
// These templates provide consistent, efficient implementations
// that can be shared across SHA-2, ChaCha20, and other crypto modules

namespace BitOps
{
    // Rotate right (circular shift right)
    template<typename T>
    static FORCE_INLINE T ROTR(T x, UINT32 n)
    {
        constexpr UINT32 bits = sizeof(T) * 8;
        return (x >> n) | (x << (bits - n));
    }

    // Rotate left (circular shift left)
    template<typename T>
    static FORCE_INLINE T ROTL(T x, UINT32 n)
    {
        constexpr UINT32 bits = sizeof(T) * 8;
        return (x << n) | (x >> (bits - n));
    }

    // Convenience aliases - generic templates compile to identical code
    static FORCE_INLINE UINT32 ROTR32(UINT32 x, UINT32 n) { return ROTR(x, n); }
    static FORCE_INLINE UINT32 ROTL32(UINT32 x, UINT32 n) { return ROTL(x, n); }
    static FORCE_INLINE UINT64 ROTR64(UINT64 x, UINT32 n) { return ROTR(x, n); }
    static FORCE_INLINE UINT64 ROTL64(UINT64 x, UINT32 n) { return ROTL(x, n); }
}
