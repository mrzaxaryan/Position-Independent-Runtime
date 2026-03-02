#pragma once

/**
 * @file tls_hash.h
 * @brief TLS 1.3 transcript hash for handshake verification
 *
 * @details Accumulates handshake messages and computes the transcript hash
 * used in TLS 1.3 key derivation and Finished message verification.
 * Supports SHA-256 and SHA-384 depending on the negotiated cipher suite.
 *
 * @see RFC 8446 Section 4.4.1 — Transcript Hash
 *      https://datatracker.ietf.org/doc/html/rfc8446#section-4.4.1
 */

#include "core/core.h"
#include "runtime/network/tls/tls_buffer.h"

/// TLS transcript hash accumulator for handshake verification
class TlsHash
{
private:
	TlsBuffer cache;
public:
	TlsHash() = default;
	~TlsHash() = default;

	// Non-copyable
	TlsHash(const TlsHash &) = delete;
	TlsHash &operator=(const TlsHash &) = delete;

	// Movable
	TlsHash(TlsHash &&) = default;
	TlsHash &operator=(TlsHash &&) = default;

	// Stack-only
	VOID *operator new(USIZE) = delete;
	VOID operator delete(VOID *) = delete;

	/**
	 * @brief Clears the accumulated handshake data, resetting the transcript hash
	 */
	VOID Reset();

	/**
	 * @brief Appends handshake message data to the transcript hash accumulator
	 * @param buffer Data to append
	 *
	 * @see RFC 8446 Section 4.4.1 — Transcript Hash
	 *      https://datatracker.ietf.org/doc/html/rfc8446#section-4.4.1
	 */
	VOID Append(Span<const CHAR> buffer);

	/**
	 * @brief Computes the transcript hash from accumulated data
	 * @param out Output span; size selects the hash algorithm
	 *            (32 = SHA-256, 48 = SHA-384)
	 *
	 * @see RFC 8446 Section 4.4.1 — Transcript Hash
	 *      https://datatracker.ietf.org/doc/html/rfc8446#section-4.4.1
	 */
	VOID GetHash(Span<CHAR> out);
};
