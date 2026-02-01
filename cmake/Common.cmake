# =============================================================================
# Common.cmake - Shared Configuration
# =============================================================================

include_guard(GLOBAL)

# =============================================================================
# Configuration Options
# =============================================================================
set(ARCHITECTURE "x86_64" CACHE STRING "Target architecture: i386, x86_64, armv7a, aarch64")
set(PLATFORM "windows" CACHE STRING "Target platform: windows, linux, uefi")
set(BUILD_TYPE "release" CACHE STRING "Build type: debug, release")
set(OPTIMIZATION_LEVEL "" CACHE STRING "Optimization level override")

# Normalize to lowercase
string(TOLOWER "${ARCHITECTURE}" CPPPIC_ARCH)
string(TOLOWER "${PLATFORM}" CPPPIC_PLATFORM)
string(TOLOWER "${BUILD_TYPE}" CPPPIC_BUILD_TYPE)

# Validate
set(_valid_arch i386 x86_64 armv7a aarch64)
set(_valid_plat windows linux uefi)
if(NOT CPPPIC_ARCH IN_LIST _valid_arch)
    message(FATAL_ERROR "Invalid ARCHITECTURE: ${ARCHITECTURE}")
endif()
if(NOT CPPPIC_PLATFORM IN_LIST _valid_plat)
    message(FATAL_ERROR "Invalid PLATFORM: ${PLATFORM}")
endif()
if(NOT CPPPIC_BUILD_TYPE MATCHES "^(debug|release)$")
    message(FATAL_ERROR "Invalid BUILD_TYPE: ${BUILD_TYPE}")
endif()

# Output directory
set(CPPPIC_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/${CPPPIC_BUILD_TYPE}/${CPPPIC_PLATFORM}/${CPPPIC_ARCH}")

# Optimization level
if(OPTIMIZATION_LEVEL STREQUAL "")
    if(CPPPIC_BUILD_TYPE STREQUAL "debug")
        set(CPPPIC_OPT_LEVEL "Og")
    else()
        set(CPPPIC_OPT_LEVEL "O3")
    endif()
else()
    set(CPPPIC_OPT_LEVEL "${OPTIMIZATION_LEVEL}")
endif()

# =============================================================================
# Architecture Defines (generated dynamically)
# =============================================================================
string(TOUPPER "${CPPPIC_ARCH}" _arch_upper)
string(TOUPPER "${CPPPIC_PLATFORM}" _plat_upper)
set(CPPPIC_DEFINES
    ARCHITECTURE_${_arch_upper}
    PLATFORM_${_plat_upper}
    PLATFORM_${_plat_upper}_${_arch_upper}
)
if(CPPPIC_BUILD_TYPE STREQUAL "debug")
    list(APPEND CPPPIC_DEFINES DEBUG)
endif()

# =============================================================================
# Base Compiler Flags
# =============================================================================
set(CPPPIC_BASE_FLAGS
    -std=c++23 -fno-jump-tables -Werror -Wall -Wextra
    -Wno-gnu-string-literal-operator-template -Qn -nostdlib
    -fno-ident -fno-exceptions -fno-rtti -fno-stack-check
    -ffunction-sections -fdata-sections -fno-builtin -fshort-wchar
)

# Architecture-specific flags
if(CPPPIC_ARCH MATCHES "^(i386|x86_64)$")
    list(APPEND CPPPIC_BASE_FLAGS -mno-stack-arg-probe -msoft-float)
elseif(CPPPIC_ARCH MATCHES "^(armv7a|aarch64)$")
    list(APPEND CPPPIC_BASE_FLAGS -mno-implicit-float)
endif()

# Build type flags
if(CPPPIC_BUILD_TYPE STREQUAL "debug")
    list(APPEND CPPPIC_BASE_FLAGS -g3 -fno-omit-frame-pointer "-${CPPPIC_OPT_LEVEL}" -ferror-limit=200)
else()
    list(APPEND CPPPIC_BASE_FLAGS
        -fno-omit-frame-pointer -fno-asynchronous-unwind-tables -fno-unwind-tables
        -flto=full -finline-functions -funroll-loops -fwhole-program-vtables
        "-${CPPPIC_OPT_LEVEL}"
    )
endif()

# =============================================================================
# Base Linker Flags
# =============================================================================
set(CPPPIC_BASE_LINK_FLAGS -fuse-ld=lld -nostdlib)

# =============================================================================
# Source Collection
# =============================================================================
set(CPPPIC_INCLUDE_PATHS
    ${CMAKE_SOURCE_DIR}/ ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/bal/ ${CMAKE_SOURCE_DIR}/include/bal/core/
    ${CMAKE_SOURCE_DIR}/include/bal/algorithms/ ${CMAKE_SOURCE_DIR}/include/bal/types/
    ${CMAKE_SOURCE_DIR}/include/bal/types/numeric/ ${CMAKE_SOURCE_DIR}/include/bal/types/embedded/
    ${CMAKE_SOURCE_DIR}/include/bal/types/network/ ${CMAKE_SOURCE_DIR}/include/bal/string/
    ${CMAKE_SOURCE_DIR}/include/pal/ ${CMAKE_SOURCE_DIR}/include/pal/memory/
    ${CMAKE_SOURCE_DIR}/include/pal/io/ ${CMAKE_SOURCE_DIR}/include/pal/network/
    ${CMAKE_SOURCE_DIR}/include/pal/system/
    ${CMAKE_SOURCE_DIR}/include/ral/ ${CMAKE_SOURCE_DIR}/include/ral/crypt/
    ${CMAKE_SOURCE_DIR}/include/ral/network/ ${CMAKE_SOURCE_DIR}/include/ral/network/tls/
    ${CMAKE_SOURCE_DIR}/tests/
)

file(GLOB_RECURSE CPPPIC_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cc)
file(GLOB_RECURSE CPPPIC_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h ${CMAKE_SOURCE_DIR}/tests/*.h)

# =============================================================================
# Post-Build Commands
# =============================================================================
function(cpppic_add_postbuild target_name)
    set(_exe "${CPPPIC_OUTPUT_DIR}/output${CPPPIC_EXT}")
    set(_bin "${CPPPIC_OUTPUT_DIR}/output.bin")
    set(_map "${CPPPIC_OUTPUT_DIR}/output.map.txt")

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Build SUCCEEDED: ${_exe}"
        COMMAND llvm-objdump -d -s -h -j .text ${_exe} > "${CPPPIC_OUTPUT_DIR}/output.txt"
        COMMAND llvm-objcopy "--dump-section=.text=${_bin}" ${_exe}
        COMMAND llvm-strings ${_exe} > "${CPPPIC_OUTPUT_DIR}/output.strings.txt"
        COMMAND ${CMAKE_COMMAND} -DPIC_FILE=${_bin} -DBASE64_FILE="${CPPPIC_OUTPUT_DIR}/output.b64.txt"
            -P "${CMAKE_SOURCE_DIR}/cmake/Base64Encode.cmake"
        COMMAND ${CMAKE_COMMAND} -DMAP_FILE=${_map}
            -P "${CMAKE_SOURCE_DIR}/cmake/VerifyPICMode.cmake"
        COMMENT "Generating PIC blob..."
        VERBATIM
    )
endfunction()
