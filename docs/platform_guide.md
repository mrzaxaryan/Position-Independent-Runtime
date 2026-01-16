# Platform Implementation Guide

This guide provides detailed information about platform-specific implementations in CPP-PIC.

## Table of Contents

1. [Windows Implementation](#windows-implementation)
2. [Linux Implementation](#linux-implementation)
3. [UEFI Implementation](#uefi-implementation)
4. [Adding New Platforms](#adding-new-platforms)

---

## Windows Implementation

### Architecture Support

| Architecture | Target Triple | Status |
|-------------|---------------|--------|
| i386 | `i386-pc-windows-gnu` | ✅ Supported |
| x86_64 | `x86_64-pc-windows-gnu` | ✅ Supported |
| armv7a | `armv7a-pc-windows-gnu` | ✅ Supported |
| aarch64 | `aarch64-pc-windows-gnu` | ✅ Supported |

### Initialization Sequence

1. **Entry Point** - `_start()` in [src/start.cc](../src/start.cc)
2. **Platform Init** - `Initialize()` in [src/runtime/platform/windows/platform.windows.cc](../src/runtime/platform/windows/platform.windows.cc)
3. **PEB Location** - `GetPEB()` in [src/runtime/platform/windows/peb.cc](../src/runtime/platform/windows/peb.cc)
4. **Module Enumeration** - Walk `PEB->Ldr->InMemoryOrderModuleList`
5. **API Resolution** - Parse PE exports, hash-based lookup

### Key Components

#### PEB Walking ([peb.cc](../src/runtime/platform/windows/peb.cc))

```cpp
// Locate Process Environment Block
PEB* peb = GetPEB();

// Access loaded modules
PEB_LDR_DATA* ldr = peb->Ldr;
LIST_ENTRY* moduleList = &ldr->InMemoryOrderModuleList;
```

**PEB Structure**:
- **ImageBaseAddress** - Base of current executable
- **Ldr** - Loader data (loaded modules)
- **ProcessParameters** - Command line, environment

#### PE Parsing ([pe.cc](../src/runtime/platform/windows/pe.cc))

```cpp
// Find export by hash
FARPROC GetExportByHash(HMODULE module, UINT32 hash);
```

**Process**:
1. Read DOS header → PE header
2. Locate Export Directory
3. Iterate exports, hash names (DJB2)
4. Return RVA when hash matches

#### API Resolution

**ntdll.dll** ([ntdll.cc](../src/runtime/platform/windows/ntdll.cc)):
- `NtAllocateVirtualMemory` - Allocate memory
- `NtFreeVirtualMemory` - Free memory
- `NtTerminateProcess` - Exit process

**kernel32.dll** ([kernel32.cc](../src/runtime/platform/windows/kernel32.cc)):
- `GetStdHandle` - Get console handles
- `WriteConsoleW` - Write wide strings to console

### Memory Allocation

```cpp
// Direct syscall to ntdll
NtAllocateVirtualMemory(
    GetCurrentProcess(),
    &baseAddress,
    0,
    &size,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_READWRITE
);
```

### Console Output

```cpp
// Get stdout handle
HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);

// Write wide string
WriteConsoleW(stdOut, buffer, length, &written, NULL);
```

### Linker Configuration

```cmake
# Merge data into code
/MERGE:.rdata=.text

# Custom entry point
/Entry:_start

# Function ordering (i386 only)
/ORDER:@orderfile.txt

# i386-specific
/BASE:0x400000
/FILEALIGN:0x1000
```

---

## Linux Implementation

### Architecture Support

| Architecture | Target Triple | Syscall ABI | Status |
|-------------|---------------|-------------|--------|
| i386 | `i386-unknown-linux-gnu` | int 0x80 | ✅ Supported |
| x86_64 | `x86_64-unknown-linux-gnu` | syscall | ✅ Supported |
| armv7a | `armv7a-unknown-linux-gnueabi` | SVC 0 | ✅ Supported |
| aarch64 | `aarch64-unknown-linux-gnu` | SVC 0 | ✅ Supported |

### Initialization Sequence

1. **Entry Point** - `_start()` in [src/start.cc](../src/start.cc)
2. **Platform Init** - `Initialize()` in arch-specific files
3. **No Dynamic Linking** - Direct syscalls only

### Syscall Interface

Each architecture has unique syscall numbers and calling conventions:

#### x86_64 ([platform.linux.x86_64.cc](../src/runtime/platform/linux/platform.linux.x86_64.cc))

```cpp
// Syscall numbers
#define SYS_write 1
#define SYS_mmap 9
#define SYS_munmap 11
#define SYS_exit 60

// Calling convention: rax, rdi, rsi, rdx, r10, r8, r9
```

#### i386 ([platform.linux.i386.cc](../src/runtime/platform/linux/platform.linux.i386.cc))

```cpp
// Syscall numbers
#define SYS_exit 1
#define SYS_write 4
#define SYS_mmap2 192
#define SYS_munmap 91

// Calling convention: eax, ebx, ecx, edx, esi, edi, ebp
// Uses int 0x80
```

#### aarch64 ([platform.linux.aarch64.cc](../src/runtime/platform/linux/platform.linux.aarch64.cc))

```cpp
// Syscall numbers
#define SYS_write 64
#define SYS_mmap 222
#define SYS_munmap 215
#define SYS_exit 93

// Calling convention: x8 (syscall #), x0-x5 (args)
// Uses SVC 0
```

#### armv7a ([platform.linux.armv7a.cc](../src/runtime/platform/linux/platform.linux.armv7a.cc))

```cpp
// Syscall numbers
#define SYS_exit 1
#define SYS_write 4
#define SYS_mmap2 192
#define SYS_munmap 91

// Calling convention: r7 (syscall #), r0-r6 (args)
// Uses SVC 0
```

### Memory Allocation

```cpp
// mmap syscall
void* addr = mmap(
    NULL,           // Let kernel choose address
    size,           // Size
    PROT_READ | PROT_WRITE,  // Protection
    MAP_PRIVATE | MAP_ANONYMOUS,  // Flags
    -1,             // fd (ignored for anonymous)
    0               // offset
);
```

### Console Output

```cpp
// write syscall to stdout (fd=1)
write(1, buffer, length);
```

### Linker Script

Custom linker script ([linker.script](../linker.script)) merges sections:

```ld
SECTIONS
{
    .text : {
        *(.text .text.*)
        *(.rodata .rodata.*)  /* Merge read-only data */
        *(.bss .bss.*)        /* Merge uninitialized data */
    }
}
```

---

## UEFI Implementation

### Architecture Support

| Architecture | Target Triple | Status |
|-------------|---------------|--------|
| i386 | `i386-unknown-windows` | ⚠️ Fallback |
| x86_64 | `x86_64-unknown-uefi` | ✅ Official |
| aarch64 | `aarch64-unknown-windows` | ⚠️ Fallback |

### Initialization Sequence

1. **Entry Point** - `EfiMain(EFI_HANDLE, EFI_SYSTEM_TABLE*)` in [src/start.cc](../src/start.cc)
2. **Platform Init** - `Initialize(&envData)` in [src/runtime/platform/uefi/platform.uefi.cc](../src/runtime/platform/uefi/platform.uefi.cc)
3. **Boot Services** - Access via System Table

### UEFI System Table

```cpp
typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16* FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_INPUT_INTERFACE* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_INTERFACE* ConOut;  // Console output
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_INTERFACE* StdErr;
    EFI_RUNTIME_SERVICES* RuntimeServices;
    EFI_BOOT_SERVICES* BootServices;           // Memory, events, protocols
    // ...
} EFI_SYSTEM_TABLE;
```

### Memory Allocation

```cpp
// AllocatePool via Boot Services
EFI_STATUS status = gBS->AllocatePool(
    EfiLoaderData,  // Memory type
    size,           // Size
    &buffer         // Output pointer
);
```

### Console Output

```cpp
// Simple Text Output Protocol
gST->ConOut->OutputString(
    gST->ConOut,
    L"Hello, UEFI!\r\n"
);
```

### Testing with QEMU

```powershell
# Build and run UEFI
.\scripts\run-uefi-qemu.ps1 -Architecture x86_64

# Script creates FAT32 disk image with EFI\BOOT\BOOTX64.EFI
# Launches QEMU with OVMF firmware
```

### Linker Configuration

```cmake
# UEFI subsystem
/SUBSYSTEM:EFI_APPLICATION

# Entry point
/Entry:EfiMain

# No CRT
/NODEFAULTLIB
```

---

## Adding New Platforms

### 1. Define Platform Constants

In [include/runtime/platform/primitives/primitives.h](../include/runtime/platform/primitives/primitives.h):

```cpp
#define PLATFORM_MYOS 1

#if defined(__myos__)
    #define CURRENT_PLATFORM PLATFORM_MYOS
#endif
```

### 2. Create Platform Headers

Create `include/runtime/platform/myos/`:
- `myos_types.h` - Platform-specific types
- `myos_syscalls.h` - System call interface

### 3. Implement Platform Functions

Create `src/runtime/platform/myos/`:

**platform.myos.cc**:
```cpp
#include "platform.h"

#if defined(PLATFORM_MYOS)

VOID Initialize(ENVIRONMENT_DATA* envData) {
    // Platform-specific initialization
}

VOID ExitProcess(INT32 exitCode) {
    // Exit implementation
}

#endif
```

**allocator.myos.cc**:
```cpp
#include "allocator.h"

#if defined(PLATFORM_MYOS)

VOID* Allocator::Allocate(SIZE_T size) {
    // Memory allocation
}

VOID Allocator::Free(VOID* ptr) {
    // Memory deallocation
}

#endif
```

### 4. Implement Console

Create `src/runtime/console/myos/console.myos.cc`:

```cpp
#if defined(PLATFORM_MYOS)

template<typename T>
VOID Console::Write(const T* str, SIZE_T length) {
    // Console output implementation
}

#endif
```

### 5. Update CMakeLists.txt

```cmake
# Add platform option
if(PLATFORM STREQUAL "myos")
    set(PLATFORM_MYOS 1)
    add_compile_definitions(PLATFORM_MYOS=1)

    # Add source files
    target_sources(output PRIVATE
        src/runtime/platform/myos/platform.myos.cc
        src/runtime/platform/myos/allocator.myos.cc
        src/runtime/console/myos/console.myos.cc
    )

    # Platform-specific flags
    target_compile_options(output PRIVATE
        -target ${CMAKE_TARGET_TRIPLE}
    )
endif()
```

### 6. Test

```bash
# Build for new platform
cmake -B build/myos/x86_64/debug \
      -DARCHITECTURE=x86_64 \
      -DPLATFORM=myos \
      -DBUILD_TYPE=debug

cmake --build build/myos/x86_64/debug
```

---

## Platform Comparison Matrix

| Feature | Windows | Linux | UEFI |
|---------|---------|-------|------|
| **Memory Alloc** | NtAllocateVirtualMemory | mmap | AllocatePool |
| **Memory Free** | NtFreeVirtualMemory | munmap | FreePool |
| **Console** | WriteConsoleW | write(1) | ConOut->OutputString |
| **Exit** | NtTerminateProcess | exit | Return from EfiMain |
| **API Resolution** | PEB/PE parsing | Direct syscall | Boot Services |
| **Entry Point** | `_start()` | `_start()` | `EfiMain()` |
| **Linker** | MSVC LLD | GNU LD script | MSVC LLD |
| **Dependencies** | None (PEB) | None (syscall) | System Table |

---

## Best Practices

### Platform Abstraction

1. **Define Generic Interfaces** - Keep platform logic isolated
2. **Use Conditional Compilation** - `#if defined(PLATFORM_XXX)`
3. **Delegate Implementation** - Generic files call platform-specific functions
4. **Avoid Platform Leakage** - Platform types stay in `platform/` directory

### Testing

1. **Test Each Platform** - Build and run on all supported platforms
2. **Verify PIC** - Ensure no relocations in `.text` section
3. **Check String Embedding** - Validate no `.rdata` section
4. **Validate Syscalls** - Test all syscall paths

### Documentation

1. **Document Syscall Numbers** - Vary by architecture
2. **Document ABI** - Calling conventions differ
3. **Document Limitations** - Platform-specific constraints
4. **Provide Examples** - Show platform usage

---

## Troubleshooting

### Windows

**Problem**: API resolution fails
- **Solution**: Verify DJB2 hash matches export name
- **Debug**: Print export names during initialization

**Problem**: Linker errors about .rdata
- **Solution**: Ensure `/MERGE:.rdata=.text` flag is set

### Linux

**Problem**: Illegal instruction
- **Solution**: Check syscall numbers for architecture
- **Solution**: Verify calling convention (registers)

**Problem**: Segmentation fault
- **Solution**: Check stack alignment (16-byte on x86_64)

### UEFI

**Problem**: QEMU doesn't boot
- **Solution**: Verify EFI file is at `EFI\BOOT\BOOT{arch}.EFI`
- **Solution**: Check OVMF firmware is available

**Problem**: No console output
- **Solution**: Ensure `ConOut->OutputString()` is called
- **Solution**: Add `\r\n` for line breaks

---

## References

- [Windows PE Format](https://docs.microsoft.com/en-us/windows/win32/debug/pe-format)
- [Linux System Calls](https://man7.org/linux/man-pages/man2/syscalls.2.html)
- [UEFI Specification](https://uefi.org/specifications)
- [Architecture Documentation](architecture.md)
