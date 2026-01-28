#pragma once

#include "ral.h"

class ArrayStorageTests
{
public:
	static BOOL RunAll()
	{
		BOOL allPassed = TRUE;

		Logger::Info<WCHAR>(L"Running ArrayStorage Tests..."_embed);

		// Test 1: Wide char array storage
		if (!TestWideCharArrayStorage())
		{
			allPassed = FALSE;
			Logger::Error<WCHAR>(L"  FAILED: Wide char array storage"_embed);
		}
		else
		{
			Logger::Info<WCHAR>(L"  PASSED: Wide char array storage"_embed);
		}

		// Test 2: UINT32 array storage
		if (!TestUInt32ArrayStorage())
		{
			allPassed = FALSE;
			Logger::Error<WCHAR>(L"  FAILED: UINT32 array storage"_embed);
		}
		else
		{
			Logger::Info<WCHAR>(L"  PASSED: UINT32 array storage"_embed);
		}

		// Test 4: UINT64 array storage
		if (!TestUInt64ArrayStorage())
		{
			allPassed = FALSE;
			Logger::Error<WCHAR>(L"  FAILED: UINT64 array storage"_embed);
		}
		else
		{
			Logger::Info<WCHAR>(L"  PASSED: UINT64 array storage"_embed);
		}

		// Test 5: Array indexing operator
		if (!TestArrayIndexing())
		{
			allPassed = FALSE;
			Logger::Error<WCHAR>(L"  FAILED: Array indexing"_embed);
		}
		else
		{
			Logger::Info<WCHAR>(L"  PASSED: Array indexing"_embed);
		}

		// Test 6: Pointer conversion and memory copy
		if (!TestPointerConversionAndCopy())
		{
			allPassed = FALSE;
			Logger::Error<WCHAR>(L"  FAILED: Pointer conversion and copy"_embed);
		}
		else
		{
			Logger::Info<WCHAR>(L"  PASSED: Pointer conversion and copy"_embed);
		}

		// Test 7: Compile-time constants
		if (!TestCompileTimeConstants())
		{
			allPassed = FALSE;
			Logger::Error<WCHAR>(L"  FAILED: Compile-time constants"_embed);
		}
		else
		{
			Logger::Info<WCHAR>(L"  PASSED: Compile-time constants"_embed);
		}

		if (allPassed)
		{
			Logger::Info<WCHAR>(L"All ArrayStorage tests passed!"_embed);
		}
		else
		{
			Logger::Error<WCHAR>(L"Some ArrayStorage tests failed!"_embed);
		}

		return allPassed;
	}

private:
	static BOOL TestWideCharArrayStorage()
	{
		constexpr auto storage = MakeEmbedArray(L"Test");

		// Verify size
		if (storage.Count != 5) // "Test" + null terminator
			return FALSE;

		// Verify data integrity
		constexpr WCHAR expected[] = L"Test";
		for (USIZE i = 0; i < 5; i++)
		{
			if (storage[i] != expected[i])
				return FALSE;
		}

		return TRUE;
	}

	static BOOL TestUInt32ArrayStorage()
	{
		constexpr UINT32 testData[] = {1, 2, 3, 4};
		constexpr auto storage = MakeEmbedArray(testData);

		// Verify size
		if (storage.Count != 4)
			return FALSE;

		// Print values to console
		Logger::Info<WCHAR>(L"    UINT32 values:"_embed);
		for (USIZE i = 0; i < 4; i++)
		{
			Logger::Info<WCHAR>(L"      %u"_embed, storage[i]);
		}

		// Verify data integrity
		for (USIZE i = 0; i < 4; i++)
		{
			if (storage[i] != testData[i])
				return FALSE;
		}

		return TRUE;
	}

	static BOOL TestUInt64ArrayStorage()
	{
		constexpr UINT64 testData[] = {
			0x123456789ABCDEF0ULL,
			0xFEDCBA9876543210ULL,
			0x0011223344556677ULL};
		constexpr auto storage = MakeEmbedArray(testData);

		// Verify size
		if (storage.Count != 3)
			return FALSE;

		// Verify data integrity
		for (USIZE i = 0; i < 3; i++)
		{
			if (storage[i] != testData[i])
				return FALSE;
		}

		return TRUE;
	}

	static BOOL TestArrayIndexing()
	{
		constexpr auto storage = MakeEmbedArray((const UINT32[]){100, 200, 300, 400, 500});

		// Test indexing operator
		if (storage[0] != 100) return FALSE;
		if (storage[1] != 200) return FALSE;
		if (storage[2] != 300) return FALSE;
		if (storage[3] != 400) return FALSE;
		if (storage[4] != 500) return FALSE;

		return TRUE;
	}

	static BOOL TestPointerConversionAndCopy()
	{
		constexpr UINT32 testData[] = {0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC};
		constexpr auto storage = MakeEmbedArray(testData);

		// Test pointer conversion and Memory::Copy
		UINT32 dest[3];
		Memory::Copy(dest, storage, sizeof(testData));

		for (USIZE i = 0; i < 3; i++)
		{
			if (dest[i] != testData[i])
				return FALSE;
		}

		return TRUE;
	}

	static BOOL TestCompileTimeConstants()
	{
		constexpr CHAR testData[] = "CompileTime";
		constexpr auto storage = MakeEmbedArray(testData);

		// Verify compile-time properties
		static_assert(storage.Count == 12, "Count should be 12");
		static_assert(storage.SizeBytes == 12, "SizeBytes should be 12");

		// Verify runtime behavior matches compile-time expectations
		if (storage.Count != 12)
			return FALSE;
		if (storage.SizeBytes != 12)
			return FALSE;

		return TRUE;
	}
};
