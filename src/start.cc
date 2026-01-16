/**
 * start.cc - CPP-PIC Runtime Entry Point
 */

#include "runtime.h"
#include "tests.h"

#if defined(PLATFORM_UEFI)
ENTRYPOINT _start(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	ENVIRONMENT_DATA envData;
	envData.ImageHandle = ImageHandle;
	envData.SystemTable = SystemTable;
	Initialize(&envData);
#else
ENTRYPOINT INT32 _start(VOID)
{
	ENVIRONMENT_DATA envData;
	Initialize(&envData);
#endif

	BOOL allPassed = TRUE;

	Logger::Info<WCHAR>(L"=== CPP-PIC Test Suite ==="_embed);
	Logger::Info<WCHAR>(L""_embed);

	// Run all test suites
	if (!Djb2Tests::RunAll())
		allPassed = FALSE;
	Logger::Info<WCHAR>(L""_embed);

	if (!MemoryTests::RunAll())
		allPassed = FALSE;
	Logger::Info<WCHAR>(L""_embed);

	if (!StringTests::RunAll())
		allPassed = FALSE;
	Logger::Info<WCHAR>(L""_embed);

	if (!Uint64Tests::RunAll())
		allPassed = FALSE;
	Logger::Info<WCHAR>(L""_embed);

	if (!Int64Tests::RunAll())
		allPassed = FALSE;
	Logger::Info<WCHAR>(L""_embed);

	if (!DoubleTests::RunAll())
		allPassed = FALSE;
	Logger::Info<WCHAR>(L""_embed);

	if (!StringFormatterTests::RunAll())
		allPassed = FALSE;
	Logger::Info<WCHAR>(L""_embed);

	// Final summary
	Logger::Info<WCHAR>(L"=== Test Suite Complete ==="_embed);
	if (allPassed)
	{
		Logger::Info<WCHAR>(L"ALL TESTS PASSED!"_embed);
	}
	else
	{
		Logger::Error<WCHAR>(L"SOME TESTS FAILED!"_embed);
	}

	ExitProcess(allPassed ? 0 : 1);
}
