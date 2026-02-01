# =============================================================================
# VerifyNoRdata.cmake - Verify no .rdata/.rodata/.data/.bss sections exist
# =============================================================================
# This script checks the linker map file to ensure no data sections exist
# that would break position-independence.
#
# Required parameters:
#   MAP_FILE - Path to the linker map file
# =============================================================================

include_guard(GLOBAL)

if(NOT DEFINED MAP_FILE)
    message(FATAL_ERROR "MAP_FILE must be defined")
endif()

# Map file may not exist for debug builds
if(NOT EXISTS "${MAP_FILE}")
    message(STATUS "Map file not found (expected for debug): ${MAP_FILE}")
    return()
endif()

file(READ "${MAP_FILE}" MAP_CONTENT)

# Check for .rdata/.rodata/.data/.bss sections (both input and output)
# Windows PE format uses .rdata/.data/.bss
# Linux ELF uses .rodata/.data/.bss
string(REGEX MATCH "[ \t]+[0-9a-fA-F]+:[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+H[ \t]+\\.rdata[ \t]" RDATA_IN "${MAP_CONTENT}")
string(REGEX MATCH "[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9]+[ \t]+\\.rdata[ \t]" RDATA_OUT "${MAP_CONTENT}")
string(REGEX MATCH "[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9]+[ \t]+\\.rodata[^.]" RODATA_OUT "${MAP_CONTENT}")
string(REGEX MATCH "[ \t]+[0-9a-fA-F]+:[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+H[ \t]+\\.data[ \t]" DATA_IN "${MAP_CONTENT}")
string(REGEX MATCH "[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9]+[ \t]+\\.data[ \t]" DATA_OUT "${MAP_CONTENT}")
string(REGEX MATCH "[ \t]+[0-9a-fA-F]+:[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+H[ \t]+\\.bss[ \t]" BSS_IN "${MAP_CONTENT}")
string(REGEX MATCH "[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9a-fA-F]+[ \t]+[0-9]+[ \t]+\\.bss[ \t]" BSS_OUT "${MAP_CONTENT}")

if(RDATA_IN OR RDATA_OUT OR RODATA_OUT OR DATA_IN OR DATA_OUT OR BSS_IN OR BSS_OUT)
    set(ERROR_MSG "CRITICAL: Data section detected that breaks position-independence!\n")
    if(RDATA_IN OR RDATA_OUT)
        set(ERROR_MSG "${ERROR_MSG}  - .rdata section found (Windows PE read-only data)\n")
    endif()
    if(RODATA_OUT)
        set(ERROR_MSG "${ERROR_MSG}  - .rodata section found (Linux ELF read-only data)\n")
    endif()
    if(DATA_IN OR DATA_OUT)
        set(ERROR_MSG "${ERROR_MSG}  - .data section found (initialized writable data)\n")
    endif()
    if(BSS_IN OR BSS_OUT)
        set(ERROR_MSG "${ERROR_MSG}  - .bss section found (uninitialized writable data)\n")
    endif()
    message(FATAL_ERROR
        "${ERROR_MSG}"
        "This breaks position-independence. Check for:\n"
        "  - Static/global variables (use stack allocation or EMBEDDED_* types)\n"
        "  - Uninitialized static/global variables (causes .bss section)\n"
        "  - Floating-point constants not using EMBEDDED_DOUBLE\n"
        "  - String literals not using EMBEDDED_STRING\n"
        "  - Array literals not using EMBEDDED_ARRAY\n"
        "Map file: ${MAP_FILE}"
    )
endif()

message(STATUS "Verification PASSED: No .rdata/.rodata/.data/.bss sections found")
