#include "runtime/network/tls/tls_hash.h"
#include "platform/io/logger.h"
#include "runtime/crypto/sha2.h"

/// @brief Reset the hash cache by clearing the underlying buffer

VOID TlsHash::Reset()
{
	cache.Clear();
}

/// @brief Append handshake message data to the transcript hash accumulator
/// @param buffer The data to append

VOID TlsHash::Append(Span<const CHAR> buffer)
{
	cache.Append(buffer);
}

/// @brief Compute the transcript hash from accumulated data using SHA-256 or SHA-384
/// @param out Output span; size selects the hash algorithm (32 = SHA-256, 48 = SHA-384)

VOID TlsHash::GetHash(Span<CHAR> out)
{
	if (out.Size() == SHA256_DIGEST_SIZE)
	{
		LOG_DEBUG("Computing SHA256 hash with size: %d bytes", (INT32)out.Size());
		SHA256 ctx;
		if (cache.GetSize() > 0)
		{
			LOG_DEBUG("SHA256 hash cache size: %d bytes", cache.GetSize());
			ctx.Update(Span<const UINT8>((UINT8 *)cache.GetBuffer(), cache.GetSize()));
		}
		ctx.Final(Span<UINT8, SHA256_DIGEST_SIZE>((UINT8 *)out.Data()));
	}
	else
	{
		LOG_DEBUG("Computing SHA384 hash with size: %d bytes", (INT32)out.Size());
		SHA384 ctx;
		if (cache.GetSize() > 0)
			ctx.Update(Span<const UINT8>((UINT8 *)cache.GetBuffer(), cache.GetSize()));
		ctx.Final(Span<UINT8, SHA384_DIGEST_SIZE>((UINT8 *)out.Data()));
	}
}
