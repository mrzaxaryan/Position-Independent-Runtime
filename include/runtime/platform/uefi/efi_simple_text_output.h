#pragma once

#include "uefi_types.h"

// Simple Text Output Mode
typedef struct _SIMPLE_TEXT_OUTPUT_MODE
{
    INT32 MaxMode;
    INT32 Mode;
    INT32 Attribute;
    INT32 CursorColumn;
    INT32 CursorRow;
    BOOL CursorVisible;
} SIMPLE_TEXT_OUTPUT_MODE;

// Simple Text Output Protocol function types
typedef EFI_STATUS(EFIAPI *EFI_TEXT_RESET)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    BOOL ExtendedVerification);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    WCHAR *String);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_TEST_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    WCHAR *String);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_QUERY_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN ModeNumber,
    UINTN *Columns,
    UINTN *Rows);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_SET_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN ModeNumber);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN Attribute);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN Column,
    UINTN Row);

typedef EFI_STATUS(EFIAPI *EFI_TEXT_ENABLE_CURSOR)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    BOOL Visible);

// Simple Text Output Protocol
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
    EFI_TEXT_RESET Reset;
    EFI_TEXT_STRING OutputString;
    EFI_TEXT_TEST_STRING TestString;
    EFI_TEXT_QUERY_MODE QueryMode;
    EFI_TEXT_SET_MODE SetMode;
    EFI_TEXT_SET_ATTRIBUTE SetAttribute;
    EFI_TEXT_CLEAR_SCREEN ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR EnableCursor;
    SIMPLE_TEXT_OUTPUT_MODE *Mode;
};

// Text attributes
#define EFI_BLACK        0x00
#define EFI_BLUE         0x01
#define EFI_GREEN        0x02
#define EFI_CYAN         0x03
#define EFI_RED          0x04
#define EFI_MAGENTA      0x05
#define EFI_BROWN        0x06
#define EFI_LIGHTGRAY    0x07
#define EFI_DARKGRAY     0x08
#define EFI_LIGHTBLUE    0x09
#define EFI_LIGHTGREEN   0x0A
#define EFI_LIGHTCYAN    0x0B
#define EFI_LIGHTRED     0x0C
#define EFI_LIGHTMAGENTA 0x0D
#define EFI_YELLOW       0x0E
#define EFI_WHITE        0x0F

#define EFI_BACKGROUND_BLACK     0x00
#define EFI_BACKGROUND_BLUE      0x10
#define EFI_BACKGROUND_GREEN     0x20
#define EFI_BACKGROUND_CYAN      0x30
#define EFI_BACKGROUND_RED       0x40
#define EFI_BACKGROUND_MAGENTA   0x50
#define EFI_BACKGROUND_BROWN     0x60
#define EFI_BACKGROUND_LIGHTGRAY 0x70

#define EFI_TEXT_ATTR(Foreground, Background) ((Foreground) | ((Background) << 4))
