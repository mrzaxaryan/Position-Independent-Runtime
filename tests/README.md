# Tests Directory

This directory contains test implementation files for the CPP-PIC project.

## Structure

```
tests/
├── unit/           # Unit tests for individual components
│   └── (future test .cc files can be moved here)
└── integration/    # Integration tests (future)
```

## Current Test Organization

Currently, all tests are header-based and located in:
- **Test Headers**: `include/tests/` - Test definitions and implementations
- **Test Execution**: `src/start.cc` - Main entry point that runs all test suites

## Test Suites

1. **Djb2Tests** - Hash function validation
2. **MemoryTests** - Memory operations (Copy, Zero, Compare)
3. **StringTests** - String manipulation
4. **Uint64Tests** - 64-bit unsigned arithmetic
5. **Int64Tests** - 64-bit signed arithmetic
6. **DoubleTests** - IEEE-754 floating-point operations
7. **StringFormatterTests** - Printf-style formatting

## Running Tests

Tests are automatically run when the main executable is launched:

```powershell
# Windows
.\build\windows\x86_64\debug\output.exe

# Linux (via WSL)
./build/linux/x86_64/debug/output.elf

# UEFI (via QEMU)
.\scripts\run-uefi-qemu.ps1 -Architecture x86_64
```

## Future Enhancements

As test implementations grow, consider:
- Moving test `.cc` files here (keep headers in `include/tests/`)
- Adding integration tests
- Adding benchmark tests
- Adding platform-specific test cases
