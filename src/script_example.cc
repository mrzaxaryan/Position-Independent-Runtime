/**
 * script_example.cc - PICScript Usage Examples
 *
 * Demonstrates the Lua-like State API with manual function registration.
 * NO functions are built-in - all must be registered from C++.
 * StdLib_Print outputs directly to Console (no callback needed).
 */

#include "ral/script/script.h"
#include "pal/io/logger.h"
#include "pal/io/console.h"
#include "bal/types/embedded/embedded_string.h"

// ============================================================================
// CUSTOM C++ FUNCTIONS
// ============================================================================

// Custom function: double(n) - doubles a number
script::Value Func_Double(script::FunctionContext& ctx)
{
    if (ctx.CheckArgs(1) && ctx.IsNumber(0))
    {
        return script::Value::Number(ctx.ToNumber(0) * 2);
    }
    return script::Value::Number(0);
}

// Custom function: square(n) - squares a number
script::Value Func_Square(script::FunctionContext& ctx)
{
    if (ctx.CheckArgs(1) && ctx.IsNumber(0))
    {
        INT64 n = ctx.ToNumber(0);
        return script::Value::Number(n * n);
    }
    return script::Value::Number(0);
}

// Custom function: greet(name) - prints a greeting
script::Value Func_Greet(script::FunctionContext& ctx)
{
    Console::Write<CHAR>("Hello, "_embed);
    if (ctx.CheckArgs(1) && ctx.IsString(0))
    {
        Console::Write<CHAR>(ctx.ToString(0));
    }
    else
    {
        Console::Write<CHAR>("World"_embed);
    }
    Console::Write<CHAR>("!\n"_embed);

    return script::Value::Nil();
}

// Custom function: sum(...) - sums all numeric arguments
script::Value Func_Sum(script::FunctionContext& ctx)
{
    INT64 total = 0;
    for (UINT8 i = 0; i < ctx.GetArgCount(); i++)
    {
        if (ctx.IsNumber(i))
        {
            total += ctx.ToNumber(i);
        }
    }
    return script::Value::Number(total);
}

// ============================================================================
// EXAMPLE 1: Using Standard Library
// ============================================================================

NOINLINE void Example_WithStdLib()
{
   LOG_INFO("=== Example 1: With Standard Library ===\n");
   
script::State* L = new script::State();

    // Register standard library (print, len, str, num, type, abs, min, max)
    script::OpenStdLib(*L);

    auto source = R"(print("Hello from PICScript!");
print("1 + 2 =", 1 + 2);
print("Type of 42:", type(42));
print("len(hello):", len("hello"));
)"_embed;

    L->DoString(source);
    delete L;
    LOG_INFO("\n");
}

// ============================================================================
// EXAMPLE 2: Manual Function Registration (No StdLib)
// ============================================================================

NOINLINE void Example_ManualRegistration()
{
    LOG_INFO("=== Example 2: Manual Registration Only ===");

    script::State* L = new script::State();

    // Register ONLY the functions we need - NO standard library
    L->Register("print"_embed, script::StdLib_Print);
    L->Register("double"_embed, Func_Double);
    L->Register("square"_embed, Func_Square);

    // Note: len, str, num, type are NOT available - not registered
    auto source = R"(print("Only print, double, square are available");
print("double(5) =", double(5));
print("square(4) =", square(4));
)"_embed;

    L->DoString(source);
    delete L;
}

// ============================================================================
// EXAMPLE 3: Custom Functions
// ============================================================================

NOINLINE void Example_CustomFunctions()
{
    LOG_INFO("=== Example 3: Custom Functions ===");

    script::State* L = new script::State();
    script::OpenStdLib(*L);

    // Register additional custom functions
    L->Register("greet"_embed, Func_Greet);
    L->Register("sum"_embed, Func_Sum);

    auto source = R"(greet("PICScript User");
print("sum(1,2,3,4,5) =", sum(1,2,3,4,5));
)"_embed;

    L->DoString(source);
    delete L;
}

// ============================================================================
// EXAMPLE 4: Setting Global Variables from C++
// ============================================================================

NOINLINE void Example_GlobalVariables()
{
    LOG_INFO("=== Example 4: Global Variables ===");

    script::State* L = new script::State();
    script::OpenStdLib(*L);

    // Set global variables from C++
    L->SetGlobalNumber("PI"_embed, 2, 314);
    L->SetGlobalString("version"_embed, 7, "1.0.0"_embed, 5);
    L->SetGlobalBool("debug"_embed, 5, TRUE);

    auto source = R"(print("PI (x100) =", PI);
print("Version:", version);
if (debug) {
    print("Debug mode is ON");
}
)"_embed;
    L->DoString(source);
    delete L;
}

// ============================================================================
// EXAMPLE 5: FizzBuzz
// ============================================================================

NOINLINE void Example_FizzBuzz()
{
    LOG_INFO("=== Example 5: FizzBuzz ===");

    script::State* L = new script::State();
    script::OpenStdLib(*L);

    auto source = R"(fn fizzbuzz(n) {
    for (var i = 1; i <= n; i = i + 1) {
        if (i % 15 == 0) {
            print("FizzBuzz");
        } else if (i % 3 == 0) {
            print("Fizz");
        } else if (i % 5 == 0) {
            print("Buzz");
        } else {
            print(i);
        }
    }
}
fizzbuzz(15);
)"_embed;
    L->DoString(source);
    delete L;
}

// ============================================================================
// EXAMPLE 6: Recursive Functions
// ============================================================================

NOINLINE void Example_Recursion()
{
    LOG_INFO("=== Example 6: Recursive Functions ===");

    script::State* L = new script::State();
    script::OpenStdLib(*L);

    auto source = R"(fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
for (var i = 1; i <= 10; i = i + 1) {
    print("factorial(", i, ") =", factorial(i));
}
)"_embed;
    L->DoString(source);
    delete L;
}

// ============================================================================
// EXAMPLE 7: Error Handling
// ============================================================================

NOINLINE void Example_ErrorHandling()
{
    LOG_INFO("=== Example 7: Error Handling ===");

    script::State* L = new script::State();
    script::OpenStdLib(*L);

    // Script with syntax error (missing semicolon)
    auto source = R"(var x = 10
print(x);
)"_embed;

    if (!L->DoString(source))
    {
        LOG_ERROR("Error: %s", L->GetError());
    }

    delete L;
}

// ============================================================================
// EXAMPLE 8: Minimal Setup (print only)
// ============================================================================

NOINLINE void Example_MinimalSetup()
{
    LOG_INFO("=== Example 8: Minimal Setup (print only) ===");

    script::State* L = new script::State();

    // Register ONLY print - absolutely minimal
    L->Register("print"_embed, script::StdLib_Print);

    auto source = R"(var x = 10;
var y = 20;
print("x + y =", x + y);
print("x * y =", x * y);
)"_embed;
    L->DoString(source);
    delete L;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

NOINLINE void RunAllScriptTests()
{
    LOG_INFO("\n");
    LOG_INFO("========================================\n");
    LOG_INFO("   PICScript Test Suite\n");
    LOG_INFO("   (No built-in functions)\n");
    LOG_INFO("========================================\n\n");
    Example_WithStdLib();
    Example_ManualRegistration();
    Example_CustomFunctions();
    Example_GlobalVariables();
    Example_FizzBuzz();
    Example_Recursion();
    Example_ErrorHandling();
    Example_MinimalSetup();

    LOG_INFO("========================================\n");
    LOG_INFO("   All Tests Complete!\n");
    LOG_INFO("========================================\n");
}
