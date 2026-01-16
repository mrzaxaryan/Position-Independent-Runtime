#pragma once

#include "primitives.h"

// UEFI uses raw 64-bit types to avoid conflicts with custom UINT64 class
typedef unsigned long long EFI_UINT64;
typedef signed long long EFI_INT64;

// UEFI uses Microsoft x64 calling convention
#if defined(ARCHITECTURE_X86_64) || defined(ARCHITECTURE_AARCH64)
typedef unsigned long long UINTN;
typedef signed long long INTN;
#else
typedef unsigned int UINTN;
typedef signed int INTN;
#endif

// UEFI Status codes
typedef UINTN EFI_STATUS;

#define EFI_SUCCESS              ((EFI_STATUS)0)
#define EFI_ERROR_BIT            ((UINTN)(1ULL << (sizeof(UINTN) * 8 - 1)))
#define EFI_ERROR(a)             (((INTN)(a)) < 0)

#define EFI_LOAD_ERROR           (EFI_ERROR_BIT | 1)
#define EFI_INVALID_PARAMETER    (EFI_ERROR_BIT | 2)
#define EFI_UNSUPPORTED          (EFI_ERROR_BIT | 3)
#define EFI_BAD_BUFFER_SIZE      (EFI_ERROR_BIT | 4)
#define EFI_BUFFER_TOO_SMALL     (EFI_ERROR_BIT | 5)
#define EFI_NOT_READY            (EFI_ERROR_BIT | 6)
#define EFI_DEVICE_ERROR         (EFI_ERROR_BIT | 7)
#define EFI_WRITE_PROTECTED      (EFI_ERROR_BIT | 8)
#define EFI_OUT_OF_RESOURCES     (EFI_ERROR_BIT | 9)
#define EFI_NOT_FOUND            (EFI_ERROR_BIT | 14)

// UEFI Handle types
typedef PVOID EFI_HANDLE;
typedef PVOID EFI_EVENT;

// UEFI Memory types
typedef enum
{
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

// UEFI Allocate types
typedef enum
{
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

// Physical and virtual addresses
typedef EFI_UINT64 EFI_PHYSICAL_ADDRESS;
typedef EFI_UINT64 EFI_VIRTUAL_ADDRESS;

// UEFI Table Header
typedef struct _EFI_TABLE_HEADER
{
    EFI_UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

// UEFI GUID
typedef struct _EFI_GUID
{
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8 Data4[8];
} EFI_GUID;

// Forward declarations
struct _EFI_SYSTEM_TABLE;
struct _EFI_BOOT_SERVICES;
struct _EFI_RUNTIME_SERVICES;
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef struct _EFI_RUNTIME_SERVICES EFI_RUNTIME_SERVICES;
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

// UEFI calling convention
#if defined(ARCHITECTURE_X86_64)
#define EFIAPI __attribute__((ms_abi))
#elif defined(ARCHITECTURE_AARCH64)
#define EFIAPI
#elif defined(ARCHITECTURE_I386)
#define EFIAPI __attribute__((cdecl))
#else
#define EFIAPI
#endif
