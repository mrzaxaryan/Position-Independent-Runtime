#pragma once

#include "primitives.h"

/**
 * EMBEDDED_STRING - Position-independent compile-time string literal embedding
 *
 * Eliminates .rdata section usage by materializing string literals directly in code.
 * Essential for shellcode, injection payloads, and strict PIC environments.
 *
 * COMMON USE CASES:
 *   - Shellcode & Position-Independent Code (PIC): Eliminates .rdata relocations
 *   - Kernel-Mode Drivers: Satisfies strict non-paged memory requirements
 *   - OS Development: Embedded systems and microkernels without data sections
 *   - Malware Development (Red Team/Research): Evades static string extraction
 *   - Bootloaders & Firmware: Pre-MMU environments without .rdata support
 */

// ============================================================================
// COMPILER REQUIREMENTS
// ============================================================================

/**
 * COMPILER OPTIMIZATION SUPPORT
 *
 * Tested and working: -O0, -O1, -O2, -O3, -Og, -Os, -Oz
 *
 * Implementation:
 *   The EMBEDDED_STRING class uses NOINLINE and DISABLE_OPTIMIZATION attributes
 *   to force runtime stack construction of strings. Characters are packed into
 *   UINT32 words at compile time (4 chars or 2 wchars per word) and written as
 *   immediate values, reducing instruction count by up to 4x compared to
 *   character-by-character writes.
 *
 * Build requirements:
 *   - i386: -mno-sse -mno-sse2 (disables SSE to prevent .rdata generation)
 *   - x86_64: -mno-sse4.1 -mno-sse4.2 -mno-avx -mno-avx2 (limits to SSE2)
 *   - -fno-vectorize -fno-slp-vectorize (prevents auto-vectorization)
 *
 * Verification:
 *   - No .rdata section in final binary
 *   - No string literals or floating-point constants in .rdata
 *   - All strings embedded as immediate values in .text section
 */

// ============================================================================
// INDEX SEQUENCE (Binary-split for O(log n) template depth)
// ============================================================================

template <USIZE... Is>
struct IndexSeq
{
};

template <typename, typename>
struct ConcatSeq;

template <USIZE... Is1, USIZE... Is2>
struct ConcatSeq<IndexSeq<Is1...>, IndexSeq<Is2...>>
{
    using type = IndexSeq<Is1..., (sizeof...(Is1) + Is2)...>;
};

template <USIZE N>
struct MakeIndexSeqImpl
{
    using type = typename ConcatSeq<
        typename MakeIndexSeqImpl<N / 2>::type,
        typename MakeIndexSeqImpl<N - N / 2>::type>::type;
};

template <>
struct MakeIndexSeqImpl<0>
{
    using type = IndexSeq<>;
};

template <>
struct MakeIndexSeqImpl<1>
{
    using type = IndexSeq<0>;
};

template <USIZE N>
using MakeIndexSeq = typename MakeIndexSeqImpl<N>::type;

// ============================================================================
// CHARACTER TYPE CONSTRAINT
// ============================================================================

// Restricts template to char or wchar_t
template <typename TChar>
concept TCHAR = __is_same_as(TChar, CHAR) || __is_same_as(TChar, WCHAR);

// ============================================================================
// EMBEDDED_STRING CLASS
// ============================================================================

template <TCHAR TChar, TChar... Cs>
class EMBEDDED_STRING
{
private:
    static constexpr USIZE N = sizeof...(Cs) + 1; // Includes null terminator
    static constexpr USIZE CharSize = sizeof(TChar);
    static constexpr USIZE CharsPerWord = sizeof(UINT32) / CharSize; // 4 for char, 2 for wchar_t
    static constexpr USIZE NumWords = (N + CharsPerWord - 1) / CharsPerWord;

    // Pad to word boundary for safe word writes
    static constexpr USIZE AllocN = NumWords * CharsPerWord;

    // Aligned for word access
    alignas(UINT32) TChar data[AllocN];

    /**
     * Compute packed word value at compile time
     * Packs CharsPerWord characters into a single UINT32
     */
    template <USIZE WordIndex>
    static consteval UINT32 GetPackedWord() noexcept
    {
        constexpr TChar chars[N] = {Cs..., TChar(0)};
        UINT32 result = 0;
        constexpr USIZE base = WordIndex * CharsPerWord;

        for (USIZE i = 0; i < CharsPerWord; ++i)
        {
            USIZE idx = base + i;
            TChar c = (idx < N) ? chars[idx] : TChar(0); // Zero-pad beyond N
            if constexpr (CharSize == 1)
            {
                result |= static_cast<UINT32>(static_cast<UINT8>(c)) << (i * 8);
            }
            else // wchar_t (2 bytes)
            {
                result |= static_cast<UINT32>(static_cast<UINT16>(c)) << (i * 16);
            }
        }
        return result;
    }

    /**
     * Write all packed words using fold expression
     * Each GetPackedWord<Is>() is a compile-time constant, becoming an immediate value
     */
    template <USIZE... Is>
    NOINLINE DISABLE_OPTIMIZATION void WritePackedWords(IndexSeq<Is...>) noexcept
    {
        UINT32 *dst = reinterpret_cast<UINT32 *>(data);
        ((dst[Is] = GetPackedWord<Is>()), ...);
    }

public:
    static constexpr USIZE Length() noexcept { return N - 1; } // Excludes null terminator

    /**
     * Runtime Constructor - Writes string as packed UINT32 words
     *
     * Using NOINLINE and DISABLE_OPTIMIZATION to prevent:
     * 1. Compile-time constant folding
     * 2. SSE vectorization
     * 3. Merging into .rdata section
     *
     * Instead of writing characters one by one, this packs 4 chars (or 2 wchars)
     * into a UINT32 and writes them at once, reducing instruction count by up to 4x.
     * Each word value is computed at compile time and becomes an immediate operand.
     */
    NOINLINE DISABLE_OPTIMIZATION EMBEDDED_STRING() noexcept : data{}
    {
        WritePackedWords(MakeIndexSeq<NumWords>{});
    }

    /**
     * Implicit conversion to const pointer
     *
     * Zero-cost conversion with no relocations or runtime overhead.
     */
    constexpr operator const TChar *() const noexcept
    {
        return data;
    }

    /**
     * Array subscript operator for direct character access
     */
    constexpr const TChar &operator[](USIZE index) const noexcept
    {
        return data[index];
    }
};

// ============================================================================
// USER-DEFINED LITERAL OPERATOR
// ============================================================================

/**
 * Literal suffix for compile-time string embedding
 *
 * Usage:
 *   auto str = "Hello"_embed;       // char version
 *   auto wstr = L"Hello"_embed;     // wchar_t version
 *
 * The compiler expands the string literal into a character parameter pack,
 * and the EMBEDDED_STRING constructor materializes it at runtime on the stack.
 * No longer consteval to allow runtime construction.
 */
template <TCHAR TChar, TChar... Chars>
FORCE_INLINE auto operator""_embed() noexcept
{
    return EMBEDDED_STRING<TChar, Chars...>{};
}