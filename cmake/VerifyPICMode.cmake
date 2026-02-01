# =============================================================================
# VerifyPICMode.cmake - Verify no data sections exist (run via cmake -P)
# =============================================================================
# Usage: cmake -DMAP_FILE=<path> -P VerifyPICMode.cmake
# =============================================================================

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED MAP_FILE)
    message(FATAL_ERROR "MAP_FILE is required")
endif()

if(NOT EXISTS "${MAP_FILE}")
    message(STATUS "Map file not found (expected for debug builds): ${MAP_FILE}")
    return()
endif()

file(READ "${MAP_FILE}" _content)

# Forbidden sections: .rdata (PE), .rodata (ELF), .data, .bss
set(_sections rdata rodata data bss)
set(_found)

foreach(_sec ${_sections})
    # Match both input sections (addr:offset sizeH) and output sections (vma size)
    if(_content MATCHES "[ \t]+[0-9a-fA-F]+[:\t ]+[0-9a-fA-F]+[H\t ]+[0-9a-fA-F]*[ \t]+\\.${_sec}[ \t\n]")
        list(APPEND _found ".${_sec}")
    endif()
endforeach()

if(_found)
    list(JOIN _found ", " _list)
    message(FATAL_ERROR
        "CRITICAL: Data sections break position-independence!\n"
        "Found: ${_list}\n\n"
        "Common causes:\n"
        "  - Static/global variables (use stack or EMBEDDED_* types)\n"
        "  - String literals (use EMBEDDED_STRING)\n"
        "  - Floating-point constants (use EMBEDDED_DOUBLE)\n"
        "  - Array literals (use EMBEDDED_ARRAY)\n\n"
        "Map file: ${MAP_FILE}"
    )
endif()

message(STATUS "PIC verification passed: no forbidden sections")
