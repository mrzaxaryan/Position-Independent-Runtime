/**
 * Tests.h - Test Utilities and Macros
 *
 * Provides convenience macros for running tests with pass/fail logging.
 */

#pragma once

#include "pal/io/logger.h"

/**
 * RUN_TEST - Convenience macro for running tests with pass/fail logging
 *
 * Executes a test function and logs the result. Updates the allPassed variable
 * if the test fails.
 *
 * USAGE:
 *   BOOL allPassed = TRUE;
 *   RUN_TEST(allPassed, TestFunction, "Test description");
 *
 * @param allPassedVar - Boolean variable to track overall test status
 * @param testFunc     - Test function to execute (must return BOOL)
 * @param description  - Human-readable description of the test
 */
#define RUN_TEST(allPassedVar, testFunc, description) \
	do { \
		if (!(testFunc())) { \
			allPassedVar = FALSE; \
			LOG_ERROR("  FAILED: " description); \
		} else { \
			LOG_INFO("  PASSED: " description); \
		} \
	} while (0)
