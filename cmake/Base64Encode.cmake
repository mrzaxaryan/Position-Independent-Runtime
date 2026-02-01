# =============================================================================
# Base64Encode.cmake - Cross-platform base64 encoding script
# =============================================================================
# Encodes a binary file to base64 format.
#
# Required parameters:
#   PIC_FILE    - Path to input binary file
#   BASE64_FILE - Path to output base64 file
# =============================================================================

include_guard(GLOBAL)

if(NOT DEFINED PIC_FILE OR NOT DEFINED BASE64_FILE)
    message(FATAL_ERROR "PIC_FILE and BASE64_FILE required")
endif()

if(NOT EXISTS "${PIC_FILE}")
    message(FATAL_ERROR "Input file not found: ${PIC_FILE}")
endif()

if(WIN32)
    execute_process(
        COMMAND certutil -encodehex -f "${PIC_FILE}" "${BASE64_FILE}" 0x40000001
        RESULT_VARIABLE RES
    )
else()
    # Try GNU base64 first, fallback to BSD
    execute_process(
        COMMAND base64 -w 0 "${PIC_FILE}"
        OUTPUT_FILE "${BASE64_FILE}"
        ERROR_QUIET
        RESULT_VARIABLE RES
    )
    if(NOT RES EQUAL 0)
        execute_process(
            COMMAND base64 "${PIC_FILE}"
            OUTPUT_VARIABLE B64
            RESULT_VARIABLE RES
        )
        string(REPLACE "\n" "" B64 "${B64}")
        file(WRITE "${BASE64_FILE}" "${B64}")
    endif()
    file(APPEND "${BASE64_FILE}" "\n")
endif()

if(NOT RES EQUAL 0)
    message(WARNING "Base64 encoding may have failed: ${RES}")
endif()
