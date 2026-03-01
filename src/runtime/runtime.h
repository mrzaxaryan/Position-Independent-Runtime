/**
 * runtime.h - Runtime Abstraction Layer
 *
 * Application-level features.
 * Depends on CORE and PLATFORM.
 */

#pragma once

#include "platform/platform.h"

// =============================================================================
// Cryptography
// =============================================================================
#include "runtime/crypto/sha2.h"
#include "runtime/crypto/ecc.h"
#include "runtime/crypto/chacha20.h"
#include "runtime/crypto/chacha20_encoder.h"

// =============================================================================
// Networking
// =============================================================================
#include "runtime/network/dns.h"
#include "runtime/network/http.h"
#include "runtime/network/websocket.h"

// TLS
#include "runtime/network/tls/tls.h"
#include "runtime/network/tls/tls_buffer.h"
#include "runtime/network/tls/tls_cipher.h"
#include "runtime/network/tls/tls_hash.h"
#include "runtime/network/tls/tls_hkdf.h"
