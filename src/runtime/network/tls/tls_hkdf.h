#pragma once

/**
 * @file tls_hkdf.h
 * @brief HKDF (HMAC-based Key Derivation Function) for TLS 1.3 key schedule
 *
 * @details Implements HKDF-Extract and HKDF-Expand (RFC 5869) with the TLS 1.3
 * label construction (RFC 8446 Section 7.1) used to derive handshake and
 * application traffic keys from shared secrets.
 *
 * @see RFC 5869 — HMAC-based Extract-and-Expand Key Derivation Function (HKDF)
 *      https://datatracker.ietf.org/doc/html/rfc5869
 * @see RFC 8446 Section 7.1 — Key Schedule
 *      https://datatracker.ietf.org/doc/html/rfc8446#section-7.1
 */

#include "core/core.h"

/// HKDF key derivation for TLS 1.3 key schedule
class TlsHkdf
{
private:
	TlsHkdf() = delete;

	/**
	 * @brief Constructs the HKDF label structure for TLS 1.3
	 * @param label The TLS 1.3 label string (without "tls13 " prefix)
	 * @param data The context data (typically a transcript hash)
	 * @param hkdflabel Output span for the serialized HkdfLabel structure
	 * @param length The desired output length
	 * @return The total serialized label length in bytes
	 *
	 * @see RFC 8446 Section 7.1 — Key Schedule
	 *      https://datatracker.ietf.org/doc/html/rfc8446#section-7.1
	 */
	static INT32 Label(Span<const CHAR> label, Span<const UCHAR> data, Span<UCHAR> hkdflabel, UINT16 length);

public:
	/**
	 * @brief HKDF-Extract: derives a pseudorandom key from input keying material
	 * @param output Output span for the extracted pseudorandom key
	 * @param salt Optional salt value (non-secret random value)
	 * @param ikm Input keying material
	 *
	 * @see RFC 5869 Section 2.2 — Step 1: Extract
	 *      https://datatracker.ietf.org/doc/html/rfc5869#section-2.2
	 */
	static VOID Extract(Span<UCHAR> output, Span<const UCHAR> salt, Span<const UCHAR> ikm);

	/**
	 * @brief HKDF-Expand: expands a pseudorandom key to the desired length
	 * @param output Output span for the expanded key material
	 * @param secret The pseudorandom key from Extract
	 * @param info Context and application-specific information
	 *
	 * @see RFC 5869 Section 2.3 — Step 2: Expand
	 *      https://datatracker.ietf.org/doc/html/rfc5869#section-2.3
	 */
	static VOID Expand(Span<UCHAR> output, Span<const UCHAR> secret, Span<const UCHAR> info);

	/**
	 * @brief HKDF-Expand-Label for TLS 1.3 key derivation
	 * @param output Output span for the derived key material
	 * @param secret The secret to expand
	 * @param label The TLS 1.3 label (e.g., "key", "iv", "finished")
	 * @param data The context data (typically a transcript hash, or empty)
	 *
	 * @details Constructs HkdfLabel = { length, "tls13 " + label, context }
	 * and calls HKDF-Expand with the serialized label as info.
	 *
	 * @see RFC 8446 Section 7.1 — Key Schedule
	 *      https://datatracker.ietf.org/doc/html/rfc8446#section-7.1
	 */
	static VOID ExpandLabel(Span<UCHAR> output, Span<const UCHAR> secret, Span<const CHAR> label, Span<const UCHAR> data);
};