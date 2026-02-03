/**
 * script.h - PIL (Position Independent Language) Main Entry Point
 *
 * Provides a State-based API for script execution.
 * NO built-in functions - all functions must be registered from C++.
 *
 * Position-independent, no .rdata dependencies.
 *
 * Part of RAL (Runtime Abstraction Layer).
 *
 * USAGE:
 *   script::State L;
 *   L.SetOutput(MyOutputFunc);
 *
 *   // Register functions manually
 *   script::OpenStdLib(L);  // OR register individual functions
 *   L.Register("myFunc", MyCustomFunction);
 *
 *   L.DoString("print(myFunc(42));");
 */

#pragma once

// Core components
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "value.h"
#include "interpreter.h"

// State-based API
#include "state.h"
#include "stdlib.h"

namespace script
{

// ============================================================================
// NO BUILT-IN FUNCTIONS
// ============================================================================
//
// PIL has NO built-in functions by default.
// All functions must be registered manually from C++.
//
// Use OpenStdLib(L) to register the standard library:
//   - print, len, str, num, type, abs, min, max
//
// Or register functions individually:
//   L.Register("print", StdLib_Print);
//   L.Register("myFunc", MyCustomFunction);
//
// ============================================================================

} // namespace script
