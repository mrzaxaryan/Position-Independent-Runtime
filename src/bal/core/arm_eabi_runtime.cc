/**
 * arm_eabi_runtime.cc - ARM EABI Compiler Runtime Support
 *
 * Provides division, modulo, and shift operations required by the ARM EABI.
 * These functions are called implicitly by the compiler when building with -nostdlib.
 *
 * Part of BAL (Base Abstraction Layer) - Core runtime support.
 */

#include "bal/core/compiler.h"
#include "bal/types/primitives.h"

#if defined(ARCHITECTURE_ARMV7A)

// =============================================================================
// 32-bit Division Helpers
// =============================================================================

/**
 * Internal 32-bit unsigned division with optional remainder
 * Optimized with fast path for power-of-2 divisors
 */
static inline UINT32 udiv32_internal(UINT32 numerator, UINT32 denominator, UINT32 *remainder)
    {
        if (denominator == 0)
        {
            if (remainder)
                *remainder = numerator;
            return 0;
        }

        // Fast path: power of 2 divisor
        if ((denominator & (denominator - 1)) == 0)
        {
            UINT32 shift = 0;
            UINT32 temp = denominator;
            while ((temp & 1) == 0)
            {
                temp >>= 1;
                shift++;
            }
            if (remainder)
                *remainder = numerator & (denominator - 1);
            return numerator >> shift;
        }

        // Software division algorithm (binary long division)
        UINT32 quotient = 0;
        UINT32 rem = 0;

        for (INT32 i = 31; i >= 0; i--)
        {
            rem = (rem << 1) | ((numerator >> i) & 1);
            if (rem >= denominator)
            {
                rem -= denominator;
                quotient |= (1U << i);
            }
        }

        if (remainder)
            *remainder = rem;
        return quotient;
    }

// =============================================================================
// 64-bit Division Helpers
// =============================================================================

/**
 * Internal 64-bit unsigned division with quotient and remainder
 * Uses binary long division algorithm
 */
static void udiv64_internal(UINT64 numerator, UINT64 denominator,
                            UINT64 *quotient, UINT64 *remainder)
    {
        if (denominator == 0)
        {
            *quotient = 0;
            *remainder = numerator;
            return;
        }

        UINT64 q = 0;
        UINT64 r = 0;

        for (INT32 i = 63; i >= 0; i--)
        {
            r <<= 1;
            if ((numerator >> i) & 1ULL)
                r |= 1ULL;

            if (r >= denominator)
            {
                r -= denominator;
                q |= (1ULL << i);
            }
        }

        *quotient = q;
        *remainder = r;
    }

extern "C"
{
    // =========================================================================
    // ARM EABI: 32-bit Division Functions
    // =========================================================================

    /**
     * __aeabi_uidiv - Unsigned 32-bit division
     * Returns: quotient in r0
     */
    COMPILER_RUNTIME UINT32 __aeabi_uidiv(UINT32 numerator, UINT32 denominator)
    {
        return udiv32_internal(numerator, denominator, NULL);
    }

    /**
     * __aeabi_uidivmod - Unsigned 32-bit division with modulo
     * Returns: quotient in r0, remainder in r1 (packed as 64-bit)
     */
    COMPILER_RUNTIME UINT64 __aeabi_uidivmod(UINT32 numerator, UINT32 denominator)
    {
        UINT32 remainder = 0;
        UINT32 quotient = udiv32_internal(numerator, denominator, &remainder);

        // Pack: quotient (low 32 bits), remainder (high 32 bits)
        return ((UINT64)remainder << 32) | quotient;
    }

    /**
     * __aeabi_idiv - Signed 32-bit division
     * Returns: quotient in r0
     */
    COMPILER_RUNTIME INT32 __aeabi_idiv(INT32 numerator, INT32 denominator)
    {
        if (denominator == 0)
            return 0;

        // Determine result sign
        const INT32 sign_num = numerator < 0 ? -1 : 1;
        const INT32 sign_den = denominator < 0 ? -1 : 1;
        const INT32 sign_quot = sign_num * sign_den;

        // Convert to absolute values
        const UINT32 abs_num = (UINT32)(numerator < 0 ? -numerator : numerator);
        const UINT32 abs_den = (UINT32)(denominator < 0 ? -denominator : denominator);

        // Perform unsigned division
        const UINT32 quotient = udiv32_internal(abs_num, abs_den, NULL);

        // Apply sign
        return sign_quot < 0 ? -(INT32)quotient : (INT32)quotient;
    }

    /**
     * __aeabi_idivmod - Signed 32-bit division with modulo
     * Returns: quotient in r0, remainder in r1 (packed as 64-bit)
     */
    COMPILER_RUNTIME INT64 __aeabi_idivmod(INT32 numerator, INT32 denominator)
    {
        if (denominator == 0)
        {
            return ((INT64)numerator << 32) | 0;
        }

        // Determine result signs
        const INT32 sign_num = numerator < 0 ? -1 : 1;
        const INT32 sign_den = denominator < 0 ? -1 : 1;
        const INT32 sign_quot = sign_num * sign_den;

        // Convert to absolute values
        const UINT32 abs_num = (UINT32)(numerator < 0 ? -numerator : numerator);
        const UINT32 abs_den = (UINT32)(denominator < 0 ? -denominator : denominator);

        // Perform unsigned division
        UINT32 remainder = 0;
        const UINT32 quotient = udiv32_internal(abs_num, abs_den, &remainder);

        // Apply signs (remainder takes sign of numerator)
        const INT32 signed_quot = sign_quot < 0 ? -(INT32)quotient : (INT32)quotient;
        const INT32 signed_rem = sign_num < 0 ? -(INT32)remainder : (INT32)remainder;

        // Pack: quotient (low 32 bits), remainder (high 32 bits)
        return ((INT64)signed_rem << 32) | (UINT32)signed_quot;
    }

    // =========================================================================
    // ARM EABI: 64-bit Division Functions
    // =========================================================================

    /**
     * Return type for 64-bit divmod operations
     * Per ARM EABI: quotient in r0:r1, remainder in r2:r3
     */
    typedef struct {
        UINT64 quotient;
        UINT64 remainder;
    } ARM_64_uldivmod_return;

    typedef struct {
        INT64 quotient;
        INT64 remainder;
    } ARM_64_ldivmod_return;

    /**
     * __aeabi_uldivmod - Unsigned 64-bit division with modulo
     * ARM EABI calling convention:
     *   Input:  numerator in r0:r1, denominator in r2:r3
     *   Output: quotient in r0:r1, remainder in r2:r3
     */
    COMPILER_RUNTIME
    __attribute__((pcs("aapcs")))
    ARM_64_uldivmod_return __aeabi_uldivmod(UINT64 numerator, UINT64 denominator)
    {
        ARM_64_uldivmod_return result;
        udiv64_internal(numerator, denominator, &result.quotient, &result.remainder);
        return result;
    }

    /**
     * __aeabi_ldivmod - Signed 64-bit division with modulo
     * ARM EABI calling convention:
     *   Input:  numerator in r0:r1, denominator in r2:r3
     *   Output: quotient in r0:r1, remainder in r2:r3
     */
    COMPILER_RUNTIME
    __attribute__((pcs("aapcs")))
    ARM_64_ldivmod_return __aeabi_ldivmod(INT64 numerator, INT64 denominator)
    {
        ARM_64_ldivmod_return result;

        if (denominator == 0)
        {
            result.quotient = 0;
            result.remainder = numerator;
            return result;
        }

        // Determine result signs
        const INT32 sign_num = numerator < 0 ? -1 : 1;
        const INT32 sign_den = denominator < 0 ? -1 : 1;
        const INT32 sign_quot = sign_num * sign_den;

        // Convert to absolute values
        const UINT64 abs_num = (UINT64)(numerator < 0 ? -numerator : numerator);
        const UINT64 abs_den = (UINT64)(denominator < 0 ? -denominator : denominator);

        // Perform unsigned division
        UINT64 quotient, remainder;
        udiv64_internal(abs_num, abs_den, &quotient, &remainder);

        // Apply signs (remainder takes sign of numerator)
        result.quotient = sign_quot < 0 ? -(INT64)quotient : (INT64)quotient;
        result.remainder = sign_num < 0 ? -(INT64)remainder : (INT64)remainder;

        return result;
    }

    // =========================================================================
    // ARM EABI: 64-bit Shift Functions
    // =========================================================================

    /**
     * __aeabi_llsr - 64-bit logical shift right
     */
    COMPILER_RUNTIME UINT64 __aeabi_llsr(UINT64 value, INT32 shift)
    {
        if (shift < 0 || shift >= 64)
            return 0;
        return value >> shift;
    }

    /**
     * __aeabi_llsl - 64-bit logical shift left
     */
    COMPILER_RUNTIME UINT64 __aeabi_llsl(UINT64 value, INT32 shift)
    {
        if (shift < 0 || shift >= 64)
            return 0;
        return value << shift;
    }

} // extern "C"

#endif // ARCHITECTURE_ARMV7A
