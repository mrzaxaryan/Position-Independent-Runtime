#pragma once

#include "ral/script/script.h"
#include "pal/io/logger.h"

// ============================================================================
// ERROR TESTS CLASS
// ============================================================================

class ErrorTests
{
public:
    static BOOL RunAll()
    {
        BOOL allPassed = TRUE;

        LOG_INFO("Running Error Tests...");

        // Test 1: Missing semicolon error
        if (!TestMissingSemicolon())
        {
            allPassed = FALSE;
            LOG_ERROR("  FAILED: Missing semicolon error detection");
        }
        else
        {
            LOG_INFO("  PASSED: Missing semicolon error detection");
        }

        // Test 2: Undefined variable error
        if (!TestUndefinedVariable())
        {
            allPassed = FALSE;
            LOG_ERROR("  FAILED: Undefined variable error detection");
        }
        else
        {
            LOG_INFO("  PASSED: Undefined variable error detection");
        }

        // Test 3: Undefined function error
        if (!TestUndefinedFunction())
        {
            allPassed = FALSE;
            LOG_ERROR("  FAILED: Undefined function error detection");
        }
        else
        {
            LOG_INFO("  PASSED: Undefined function error detection");
        }

        // Test 4: Syntax error in expression
        if (!TestSyntaxErrorInExpression())
        {
            allPassed = FALSE;
            LOG_ERROR("  FAILED: Syntax error in expression detection");
        }
        else
        {
            LOG_INFO("  PASSED: Syntax error in expression detection");
        }

        // Test 5: Valid script should succeed
        if (!TestValidScript())
        {
            allPassed = FALSE;
            LOG_ERROR("  FAILED: Valid script execution");
        }
        else
        {
            LOG_INFO("  PASSED: Valid script execution");
        }

        // Test 6: Error message retrieval
        if (!TestErrorMessageRetrieval())
        {
            allPassed = FALSE;
            LOG_ERROR("  FAILED: Error message retrieval");
        }
        else
        {
            LOG_INFO("  PASSED: Error message retrieval");
        }

        if (allPassed)
        {
            LOG_INFO("All Error tests passed!");
        }
        else
        {
            LOG_ERROR("Some Error tests failed!");
        }

        return allPassed;
    }

private:
    static BOOL TestMissingSemicolon()
    {
        script::State* L = new script::State();
        script::OpenStdLib(*L);

        // Script with syntax error (missing semicolon)
        auto source = R"(var x = 10
print(x);
)"_embed;

        // Should return FALSE due to error
        BOOL result = !L->DoString(source);

        if (result)
        {
            LOG_INFO("    Error detected: %s", L->GetError());
        }

        delete L;
        return result;
    }

    static BOOL TestUndefinedVariable()
    {
        script::State* L = new script::State();
        script::OpenStdLib(*L);

        // Script with undefined variable
        auto source = R"(print(undefinedVar);
)"_embed;

        // Should return FALSE due to error
        BOOL result = !L->DoString(source);

        if (result)
        {
            LOG_INFO("    Error detected: %s", L->GetError());
        }

        delete L;
        return result;
    }

    static BOOL TestUndefinedFunction()
    {
        script::State* L = new script::State();
        // Note: NOT registering any functions, not even print

        // Script calling undefined function
        auto source = R"(undefinedFunc(42);
)"_embed;

        // Should return FALSE due to error
        BOOL result = !L->DoString(source);

        if (result)
        {
            LOG_INFO("    Error detected: %s", L->GetError());
        }

        delete L;
        return result;
    }

    static BOOL TestSyntaxErrorInExpression()
    {
        script::State* L = new script::State();
        script::OpenStdLib(*L);

        // Script with invalid expression
        auto source = R"(var x = 5 + + 3;
)"_embed;

        // Should return FALSE due to error
        BOOL result = !L->DoString(source);

        if (result)
        {
            LOG_INFO("    Error detected: %s", L->GetError());
        }

        delete L;
        return result;
    }

    static BOOL TestValidScript()
    {
        script::State* L = new script::State();
        script::OpenStdLib(*L);

        // Valid script should succeed
        auto source = R"(var x = 10;
var y = 20;
print("Valid script: x + y =", x + y);
)"_embed;

        BOOL result = L->DoString(source);
        delete L;
        return result;
    }

    static BOOL TestErrorMessageRetrieval()
    {
        script::State* L = new script::State();
        script::OpenStdLib(*L);

        // Script with error
        auto source = R"(var x = 10
)"_embed;

        L->DoString(source);

        // GetError should return a non-empty string
        const CHAR* error = L->GetError();
        BOOL result = (error != nullptr && error[0] != '\0');

        if (result)
        {
            LOG_INFO("    Retrieved error: %s", error);
        }

        delete L;
        return result;
    }
};
