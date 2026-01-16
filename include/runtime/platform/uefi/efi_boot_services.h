#pragma once

#include "uefi_types.h"

// Boot Services function pointer types
typedef EFI_STATUS(EFIAPI *EFI_ALLOCATE_POOL)(
    EFI_MEMORY_TYPE PoolType,
    UINTN Size,
    PVOID *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_FREE_POOL)(
    PVOID Buffer);

typedef EFI_STATUS(EFIAPI *EFI_ALLOCATE_PAGES)(
    EFI_ALLOCATE_TYPE Type,
    EFI_MEMORY_TYPE MemoryType,
    UINTN Pages,
    EFI_PHYSICAL_ADDRESS *Memory);

typedef EFI_STATUS(EFIAPI *EFI_FREE_PAGES)(
    EFI_PHYSICAL_ADDRESS Memory,
    UINTN Pages);

typedef VOID(EFIAPI *EFI_SET_MEM)(
    PVOID Buffer,
    UINTN Size,
    UINT8 Value);

typedef VOID(EFIAPI *EFI_COPY_MEM)(
    PVOID Destination,
    PVOID Source,
    UINTN Length);

typedef EFI_STATUS(EFIAPI *EFI_EXIT)(
    EFI_HANDLE ImageHandle,
    EFI_STATUS ExitStatus,
    UINTN ExitDataSize,
    WCHAR *ExitData);

typedef EFI_STATUS(EFIAPI *EFI_EXIT_BOOT_SERVICES)(
    EFI_HANDLE ImageHandle,
    UINTN MapKey);

typedef EFI_STATUS(EFIAPI *EFI_STALL)(
    UINTN Microseconds);

// Task Priority Level
typedef UINTN EFI_TPL;

typedef EFI_TPL(EFIAPI *EFI_RAISE_TPL)(
    EFI_TPL NewTpl);

typedef VOID(EFIAPI *EFI_RESTORE_TPL)(
    EFI_TPL OldTpl);

// Locate Handle search types
typedef enum
{
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

typedef EFI_STATUS(EFIAPI *EFI_LOCATE_HANDLE)(
    EFI_LOCATE_SEARCH_TYPE SearchType,
    EFI_GUID *Protocol,
    PVOID SearchKey,
    UINTN *BufferSize,
    EFI_HANDLE *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_HANDLE_PROTOCOL)(
    EFI_HANDLE Handle,
    EFI_GUID *Protocol,
    PVOID *Interface);

// EFI Boot Services Table
struct _EFI_BOOT_SERVICES
{
    EFI_TABLE_HEADER Hdr;

    // Task Priority Services
    EFI_RAISE_TPL RaiseTPL;
    EFI_RESTORE_TPL RestoreTPL;

    // Memory Services
    EFI_ALLOCATE_PAGES AllocatePages;
    EFI_FREE_PAGES FreePages;
    PVOID GetMemoryMap;
    EFI_ALLOCATE_POOL AllocatePool;
    EFI_FREE_POOL FreePool;

    // Event & Timer Services
    PVOID CreateEvent;
    PVOID SetTimer;
    PVOID WaitForEvent;
    PVOID SignalEvent;
    PVOID CloseEvent;
    PVOID CheckEvent;

    // Protocol Handler Services
    PVOID InstallProtocolInterface;
    PVOID ReinstallProtocolInterface;
    PVOID UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL HandleProtocol;
    PVOID Reserved;
    PVOID RegisterProtocolNotify;
    EFI_LOCATE_HANDLE LocateHandle;
    PVOID LocateDevicePath;
    PVOID InstallConfigurationTable;

    // Image Services
    PVOID LoadImage;
    PVOID StartImage;
    EFI_EXIT Exit;
    PVOID UnloadImage;
    EFI_EXIT_BOOT_SERVICES ExitBootServices;

    // Miscellaneous Services
    PVOID GetNextMonotonicCount;
    EFI_STALL Stall;
    PVOID SetWatchdogTimer;

    // Driver Support Services
    PVOID ConnectController;
    PVOID DisconnectController;

    // Open and Close Protocol Services
    PVOID OpenProtocol;
    PVOID CloseProtocol;
    PVOID OpenProtocolInformation;

    // Library Services
    PVOID ProtocolsPerHandle;
    PVOID LocateHandleBuffer;
    PVOID LocateProtocol;
    PVOID InstallMultipleProtocolInterfaces;
    PVOID UninstallMultipleProtocolInterfaces;

    // 32-bit CRC Services
    PVOID CalculateCrc32;

    // Miscellaneous Services
    EFI_COPY_MEM CopyMem;
    EFI_SET_MEM SetMem;
    PVOID CreateEventEx;
};
