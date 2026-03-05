/**
 * pir_tests.h - Unified PIR Test Suite Header
 *
 * This header exposes all test suite classes for the CPP-PIC runtime.
 * Include this single header to access all test functionality.
 *
 * TEST SUITES:
 *   Djb2Tests              - Hash function tests
 *   MemoryTests            - Memory operations tests
 *   StringTests            - String utility tests
 *   DoubleTests            - Floating-point tests
 *   StringFormatterTests   - Printf-style formatting tests
 *   RandomTests            - Random number generation tests
 *   SocketTests            - Socket and network tests
 *   TlsTests               - TLS 1.3 implementation tests
 *   ArrayStorageTests      - Compile-time array storage tests
 *   ShaTests               - SHA-2 hash function tests (SHA-224/256/384/512 and HMAC)
 *   Base64Tests            - Base64 encoding/decoding tests
 *   EccTests               - Elliptic Curve Cryptography tests (ECDH key exchange)
 *   DnsTests               - DNS resolution tests (DoT, DoH JSON, DoH binary)
 *   WebSocketTests         - WebSocket client implementation tests (ws:// and wss://)
 *   ResultTests            - Result<T,E> type tests
 *   IPAddressTests         - IPAddress constexpr and runtime tests
 *   UuidTests              - UUID construction, parsing, and serialization tests
 *   SizeReportTests        - Object sizeof report (sorted large to small)
 *
 * USAGE:
 *   #include "tests.h"
 *
 *   // Run all tests
 *   Djb2Tests::RunAll();
 *   MemoryTests::RunAll();
 *   ArrayStorageTests::RunAll();
 *   SocketTests::RunAll();
 *   TlsTests::RunAll();
 *   ShaTests::RunAll();
 *   Base64Tests::RunAll();
 *   EccTests::RunAll();
 *   DnsTests::RunAll();
 *   WebSocketTests::RunAll();
 *   // ... etc
 */

#pragma once

#include "core/core.h"

#include "djb2_tests.h"
#include "memory_tests.h"
#include "string_tests.h"
#include "double_tests.h"
#include "string_formatter_tests.h"
#include "prng_tests.h"
#include "random_tests.h"
#include "socket_tests.h"
#include "tls_tests.h"
#include "array_storage_tests.h"
#include "sha_tests.h"
#include "base64_tests.h"
#include "ecc_tests.h"
#include "dns_tests.h"
#include "websocket_tests.h"
#include "file_system_tests.h"
#include "result_tests.h"
#include "binary_io_tests.h"
#include "span_tests.h"
#include "ip_address_tests.h"
#include "uuid_tests.h"
#include "size_report_tests.h"

static NOINLINE VOID DiagnosticDump()
{
	// --- Diagnostic: dump EMBEDDED_STRING internals ---
	// Use raw Console::Write to bypass Logger (which itself uses EMBEDDED_STRING)
	Console::Write<CHAR>("=== EMBEDDED_STRING Diagnostic ===\n"_embed);

	// Test 1: simple ASCII string
	auto simple = "HELLO"_embed;
	const CHAR *sp = (const CHAR *)simple;
	Console::WriteFormatted<CHAR>("simple[0..4]: %02X %02X %02X %02X %02X  len=%u\n"_embed,
		(UINT32)(UINT8)sp[0], (UINT32)(UINT8)sp[1], (UINT32)(UINT8)sp[2],
		(UINT32)(UINT8)sp[3], (UINT32)(UINT8)sp[4], (UINT32)StringUtils::Length(sp));

	// Test 2: color prefix (the string that shows as '?')
	auto color = "\033[0;32m[INF] "_embed;
	const CHAR *cp = (const CHAR *)color;
	USIZE cpLen = StringUtils::Length(cp);
	Console::WriteFormatted<CHAR>("color len=%u  bytes: "_embed, (UINT32)cpLen);
	for (USIZE i = 0; i < 14 && i <= cpLen; i++)
		Console::WriteFormatted<CHAR>("%02X "_embed, (UINT32)(UINT8)cp[i]);
	Console::Write<CHAR>("\n"_embed);

	// Test 3: dump packed words directly
	Console::WriteFormatted<CHAR>("sizeof(USIZE)=%u  CharsPerWord=%u\n"_embed,
		(UINT32)sizeof(USIZE), (UINT32)(sizeof(USIZE) / sizeof(CHAR)));

	// Test 4: write the color prefix raw to see what the terminal shows
	Console::Write<CHAR>("raw color output: ["_embed);
	Console::Write(Span<const CHAR>(cp, cpLen));
	Console::Write<CHAR>("]\n"_embed);

	// Test 5: test the Logger directly
	Console::Write<CHAR>("Logger test follows:\n"_embed);
}

static BOOL RunPIRTests()
{
	BOOL allPassed = true;

	DiagnosticDump();

	LOG_INFO("=== CPP-PIC Test Suite ===");
	LOG_INFO("");

	// CORE - Result Type and Embedded Types
	RunTestSuite<SpanTests>(allPassed);
	RunTestSuite<ResultTests>(allPassed);
	RunTestSuite<DoubleTests>(allPassed);
	RunTestSuite<StringTests>(allPassed);
	RunTestSuite<IPAddressTests>(allPassed);
	RunTestSuite<UuidTests>(allPassed);

	// CORE - Data Structures, String Utilities, and Algorithms
	RunTestSuite<ArrayStorageTests>(allPassed);
	RunTestSuite<StringFormatterTests>(allPassed);
	RunTestSuite<Djb2Tests>(allPassed);
	RunTestSuite<Base64Tests>(allPassed);
	RunTestSuite<BinaryIOTests>(allPassed);
	RunTestSuite<PrngTests>(allPassed);

	// PLATFORM - Memory, System, and File I/O
	RunTestSuite<MemoryTests>(allPassed);
	RunTestSuite<RandomTests>(allPassed);
	RunTestSuite<FileSystemTests>(allPassed);

	// RAL - Cryptography
	RunTestSuite<ShaTests>(allPassed);
	RunTestSuite<EccTests>(allPassed);

	// RAL - Network
	RunTestSuite<SocketTests>(allPassed);
	RunTestSuite<TlsTests>(allPassed);
	RunTestSuite<DnsTests>(allPassed);
	RunTestSuite<WebSocketTests>(allPassed);

	// Size Report always runs last since it's just informational and doesn't test functionality
	RunTestSuite<SizeReportTests>(allPassed);
	// Final summary
	LOG_INFO("=== Test Suite Complete ===");
	if (allPassed)
		LOG_INFO("ALL TESTS PASSED!");
	else
		LOG_ERROR("SOME TESTS FAILED!");

	return allPassed;
}