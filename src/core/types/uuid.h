/**
 * @file uuid.h
 * @brief Universally Unique Identifier (UUID) Generation and Manipulation
 *
 * @details Provides UUID generation (version 4, random), parsing from string
 * representation, and serialization to the standard 8-4-4-4-12 hex format.
 *
 * @see RFC 9562 — Universally Unique IDentifiers (UUIDs)
 *      https://datatracker.ietf.org/doc/html/rfc9562
 *
 * @ingroup core
 */

#pragma once

#include "core/types/primitives.h"
#include "core/types/span.h"
#include "core/types/error.h"
#include "core/types/result.h"
#include "core/types/embedded/embedded_string.h"
#include "core/string/string.h"

/**
 * @class UUID
 * @brief 128-bit Universally Unique Identifier
 *
 * @details Stores a 128-bit UUID as a 16-byte array and provides factory
 * methods for random generation and string parsing.
 */
class UUID
{
private:
	UINT8 data[16];

public:
	VOID *operator new(USIZE) = delete;
	VOID *operator new[](USIZE) = delete;
	VOID operator delete(VOID *) = delete;
	VOID operator delete[](VOID *) = delete;
	VOID *operator new(USIZE, PVOID ptr) noexcept { return ptr; }
	VOID operator delete(VOID *, PVOID) noexcept {}

	/// @name Constructors
	/// @{

	/**
	 * @brief Default constructor — initializes to nil UUID (all zeros)
	 */
	constexpr UUID() noexcept : data{}
	{
	}

	/**
	 * @brief Construct from a 16-byte span
	 * @param bytes Exactly 16 bytes of UUID data
	 */
	constexpr UUID(Span<const UINT8, 16> bytes) noexcept : data{}
	{
		for (USIZE i = 0; i < 16; i++)
			data[i] = bytes[i];
	}

	/// @}
	/// @name Factory Methods
	/// @{

	/**
	 * @brief Parse a UUID from a string span
	 * @param str UUID string (e.g., "550e8400-e29b-41d4-a716-446655440000")
	 * @return Ok(UUID) on success, Err(Uuid_FromStringFailed) if the string
	 *         does not contain exactly 32 hex digits
	 */

    [[nodiscard]] static Result<UUID, Error> FromString(Span<const CHAR> str) noexcept { 
        UINT8 bytes[16];
        INT32 byteidx = 0;
        INT32 stridx = 0;

        while(str[stridx] != '\0' && byteidx < 16){
            if(str[stridx] == '-'){
                stridx++;
                continue;
            }
                
            for(int nibble = 0; nibble < 2; nibble++){
                CHAR c = str[stridx++];
                UINT8 value = 0;

                if(c >= '0' && c <= '9'){
                    value = c - '0';
                } else if(c >= 'a' && c <= 'f'){
                    value = 10 + (c - 'a');
                } else if(c >= 'A' && c <= 'F'){
                    value = 10 + (c - 'A');
                } else {
                    return Result<UUID, Error>::Err(Error::Uuid_FromStringFailed);
                }
                if(nibble == 0){
                    bytes[byteidx] = value << 4;
                } else {
                   bytes[byteidx] = bytes[byteidx] | value;
                }
            } byteidx++;
        }
    return byteidx == 16 ? Result<UUID, Error>::Ok(UUID(bytes)) : Result<UUID, Error>::Err(Error::Uuid_FromStringFailed);
    }

    /**
     * @param str Null-terminated UUID string (e.g., "550e8400-e29b-41d4-a716-446655440000")
     * @return Ok(UUID) on success, Err(Uuid_FromStringFailed) if the string
     *         does not contain exactly 32 hex digits
     */
    [[nodiscard]] static Result<UUID, Error> FromString(PCCHAR str) noexcept
    {
        return FromString(Span<const CHAR>(str, StringUtils::Length(str)));
    }

   
    /// @}
    /// @name Serialization
    /// @{

     /**
     * @brief Convert UUID to its standard string representation
     * @param buffer Output buffer (must be at least 37 bytes: 32 hex + 4 dashes + null)
     * @return Ok on success, Err if the buffer is too small
     */

    [[nodiscard]] Result<void, Error> ToString(Span<CHAR> buffer) const noexcept{
        if(buffer.Size() < 37){ // 36 chars + null terminator
            return Result<void, Error>::Err(Error::Uuid_ToStringFailed);
        }

        const UINT8* bytes = data;
        INT32 pos = 0;

        for(INT32 i =0; i < 16; i++){
            if(i == 4 || i == 6 || i == 8 || i == 10){
                buffer[pos++] = '-';
            }
            const CHAR* hex = "0123456789abcdef"_embed;

            buffer[pos++] = hex[bytes[i] >> 4];
            buffer[pos++] = hex[bytes[i] & 0x0F];
        }
        buffer[pos] = '\0';
        return Result<void, Error>::Ok();
    }
};
