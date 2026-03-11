# =============================================================================
# PICTransform.cmake - Acquire the pic-transform LLVM pass tool
# =============================================================================
# Acquires pic-transform (in order of preference):
#   1. Already on PATH
#   2. Build from submodule source (tools/pic-transform)
#   3. Prebuilt binary from GitHub releases
#
# The pass eliminates .rodata/.rdata/.data/.bss sections by transforming global
# constants into stack-local immediate values automatically during compilation.

include_guard(GLOBAL)

# =============================================================================
# Detect host platform for download URL / binary name
# =============================================================================
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(_PT_PLATFORM "linux-aarch64")
    else()
        set(_PT_PLATFORM "linux-x86_64")
    endif()
    set(_PT_EXT ".tar.gz")
    set(_PT_BIN_NAME "pic-transform")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(_PT_PLATFORM "macos-arm64")
    else()
        set(_PT_PLATFORM "macos-x86_64")
    endif()
    set(_PT_EXT ".tar.gz")
    set(_PT_BIN_NAME "pic-transform")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(_PT_PLATFORM "windows-aarch64")
    else()
        set(_PT_PLATFORM "windows-x86_64")
    endif()
    set(_PT_EXT ".zip")
    set(_PT_BIN_NAME "pic-transform.exe")
else()
    message(FATAL_ERROR "pic-transform: unsupported host platform ${CMAKE_HOST_SYSTEM_NAME}")
endif()

# Plugin file name (platform-dependent)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(_PT_PLUGIN_NAME "PICTransform.dll")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(_PT_PLUGIN_NAME "PICTransform.dylib")
else()
    set(_PT_PLUGIN_NAME "PICTransform.so")
endif()

# =============================================================================
# Strategy 1: Already on PATH
# =============================================================================
find_program(PIC_TRANSFORM_EXECUTABLE pic-transform)

if(PIC_TRANSFORM_EXECUTABLE)
    message(STATUS "pic-transform: using system binary (${PIC_TRANSFORM_EXECUTABLE})")
    set(PIC_TRANSFORM_TARGET "")
    return()
endif()

# =============================================================================
# Strategy 2: Build from submodule source
# =============================================================================
# Builds pic-transform from tools/pic-transform if the submodule is populated
# and LLVM development files are available. Supports both plugin (.so/.dylib)
# and standalone executable modes.
#
# Override LLVM location: -DPIC_TRANSFORM_LLVM_DIR=/path/to/llvm/lib/cmake/llvm

set(_PT_SUBMODULE_DIR "${PIR_ROOT_DIR}/tools/pic-transform")

if(EXISTS "${_PT_SUBMODULE_DIR}/CMakeLists.txt")
    # ── Locate LLVM cmake config ────────────────────────────────────────────
    if(NOT DEFINED PIC_TRANSFORM_LLVM_DIR)
        # Resolve clang++ to absolute path (handles bare "clang++" on PATH)
        find_program(_PT_CLANG_RESOLVED "${CMAKE_CXX_COMPILER}")
        if(_PT_CLANG_RESOLVED)
            # Resolve symlinks to find actual installation
            get_filename_component(_PT_CLANG_REAL "${_PT_CLANG_RESOLVED}" REALPATH)
            get_filename_component(_PT_CLANG_DIR "${_PT_CLANG_REAL}" DIRECTORY)
            get_filename_component(_PT_LLVM_ROOT "${_PT_CLANG_DIR}/.." ABSOLUTE)

            # Search common LLVM cmake config locations
            foreach(_candidate
                "${_PT_LLVM_ROOT}/lib/cmake/llvm"
                "${_PT_LLVM_ROOT}/lib64/cmake/llvm"
            )
                if(EXISTS "${_candidate}/LLVMConfig.cmake")
                    set(PIC_TRANSFORM_LLVM_DIR "${_candidate}")
                    break()
                endif()
            endforeach()
        endif()
    endif()

    if(DEFINED PIC_TRANSFORM_LLVM_DIR AND EXISTS "${PIC_TRANSFORM_LLVM_DIR}/LLVMConfig.cmake")
        set(_PT_BUILD_DIR "${CMAKE_BINARY_DIR}/pic-transform-build")

        # Check if already built from a previous configure
        set(_PT_BUILT_BIN "${_PT_BUILD_DIR}/${_PT_BIN_NAME}")
        set(_PT_BUILT_BIN_RELEASE "${_PT_BUILD_DIR}/Release/${_PT_BIN_NAME}")
        set(_PT_BUILT_PLUGIN "${_PT_BUILD_DIR}/${_PT_PLUGIN_NAME}")
        set(_PT_BUILT_PLUGIN_RELEASE "${_PT_BUILD_DIR}/Release/${_PT_PLUGIN_NAME}")

        # Configure (only if not already configured)
        if(NOT EXISTS "${_PT_BUILD_DIR}/CMakeCache.txt")
            message(STATUS "pic-transform: configuring from source (LLVM_DIR=${PIC_TRANSFORM_LLVM_DIR})")
            execute_process(
                COMMAND ${CMAKE_COMMAND}
                    -S "${_PT_SUBMODULE_DIR}"
                    -B "${_PT_BUILD_DIR}"
                    -DLLVM_DIR=${PIC_TRANSFORM_LLVM_DIR}
                    -DCMAKE_BUILD_TYPE=Release
                    -DSTATIC_LINK=ON
                RESULT_VARIABLE _PT_CFG_RESULT
                OUTPUT_VARIABLE _PT_CFG_OUTPUT
                ERROR_VARIABLE _PT_CFG_ERROR
            )
        else()
            set(_PT_CFG_RESULT 0)
        endif()

        if(_PT_CFG_RESULT EQUAL 0)
            # Build
            message(STATUS "pic-transform: building from source...")
            execute_process(
                COMMAND ${CMAKE_COMMAND} --build "${_PT_BUILD_DIR}" --config Release
                RESULT_VARIABLE _PT_BUILD_RESULT
                OUTPUT_VARIABLE _PT_BUILD_OUTPUT
                ERROR_VARIABLE _PT_BUILD_ERROR
            )

            if(_PT_BUILD_RESULT EQUAL 0)
                # Detect what was built: prefer plugin, fall back to standalone
                if(EXISTS "${_PT_BUILT_PLUGIN}")
                    set(PIC_TRANSFORM_PLUGIN "${_PT_BUILT_PLUGIN}")
                    set(PIC_TRANSFORM_EXECUTABLE "")
                    set(PIC_TRANSFORM_TARGET "")
                    message(STATUS "pic-transform: built plugin from source (${PIC_TRANSFORM_PLUGIN})")
                    return()
                elseif(EXISTS "${_PT_BUILT_PLUGIN_RELEASE}")
                    set(PIC_TRANSFORM_PLUGIN "${_PT_BUILT_PLUGIN_RELEASE}")
                    set(PIC_TRANSFORM_EXECUTABLE "")
                    set(PIC_TRANSFORM_TARGET "")
                    message(STATUS "pic-transform: built plugin from source (${PIC_TRANSFORM_PLUGIN})")
                    return()
                elseif(EXISTS "${_PT_BUILT_BIN}")
                    set(PIC_TRANSFORM_EXECUTABLE "${_PT_BUILT_BIN}")
                    set(PIC_TRANSFORM_PLUGIN "")
                    set(PIC_TRANSFORM_TARGET "")
                    message(STATUS "pic-transform: built standalone from source (${PIC_TRANSFORM_EXECUTABLE})")
                    return()
                elseif(EXISTS "${_PT_BUILT_BIN_RELEASE}")
                    set(PIC_TRANSFORM_EXECUTABLE "${_PT_BUILT_BIN_RELEASE}")
                    set(PIC_TRANSFORM_PLUGIN "")
                    set(PIC_TRANSFORM_TARGET "")
                    message(STATUS "pic-transform: built standalone from source (${PIC_TRANSFORM_EXECUTABLE})")
                    return()
                endif()
            endif()
        endif()

        # Source build failed — log and fall through to download
        message(STATUS "pic-transform: source build failed, falling back to download")
        if(_PT_CFG_RESULT AND NOT _PT_CFG_RESULT EQUAL 0)
            message(STATUS "  configure error: ${_PT_CFG_ERROR}")
        elseif(_PT_BUILD_RESULT AND NOT _PT_BUILD_RESULT EQUAL 0)
            message(STATUS "  build error: ${_PT_BUILD_ERROR}")
        endif()
    else()
        message(STATUS "pic-transform: LLVM dev files not found, skipping source build")
    endif()
endif()

# =============================================================================
# Strategy 3: Download prebuilt from GitHub releases
# =============================================================================
set(_PT_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/pic-transform")
set(_PT_DOWNLOADED_BIN "${_PT_DOWNLOAD_DIR}/${_PT_BIN_NAME}")

if(NOT EXISTS "${_PT_DOWNLOADED_BIN}")
    set(_PT_URL "https://github.com/mrzaxaryan/pic-transform/releases/latest/download/pic-transform-${_PT_PLATFORM}${_PT_EXT}")

    message(STATUS "pic-transform: downloading from ${_PT_URL}")
    file(MAKE_DIRECTORY "${_PT_DOWNLOAD_DIR}")

    set(_PT_ARCHIVE "${_PT_DOWNLOAD_DIR}/pic-transform${_PT_EXT}")
    file(DOWNLOAD "${_PT_URL}" "${_PT_ARCHIVE}"
        STATUS _PT_DL_STATUS
        TIMEOUT 60)

    list(GET _PT_DL_STATUS 0 _PT_DL_CODE)
    if(NOT _PT_DL_CODE EQUAL 0)
        list(GET _PT_DL_STATUS 1 _PT_DL_MSG)
        message(FATAL_ERROR "pic-transform: download failed: ${_PT_DL_MSG}\n  URL: ${_PT_URL}")
    endif()

    # Extract
    if(_PT_EXT STREQUAL ".tar.gz")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${_PT_ARCHIVE}"
            WORKING_DIRECTORY "${_PT_DOWNLOAD_DIR}"
            RESULT_VARIABLE _PT_EXTRACT_RESULT)
    else()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf "${_PT_ARCHIVE}"
            WORKING_DIRECTORY "${_PT_DOWNLOAD_DIR}"
            RESULT_VARIABLE _PT_EXTRACT_RESULT)
    endif()

    if(NOT _PT_EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "pic-transform: extraction failed")
    endif()

    # Make executable on Unix
    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND EXISTS "${_PT_DOWNLOADED_BIN}")
        execute_process(COMMAND chmod +x "${_PT_DOWNLOADED_BIN}")
    endif()
endif()

if(NOT EXISTS "${_PT_DOWNLOADED_BIN}")
    message(FATAL_ERROR "pic-transform: binary not found at ${_PT_DOWNLOADED_BIN} after download")
endif()

set(PIC_TRANSFORM_EXECUTABLE "${_PT_DOWNLOADED_BIN}")
set(PIC_TRANSFORM_TARGET "")
set(PIC_TRANSFORM_PLUGIN "")
message(STATUS "pic-transform: using prebuilt binary (${PIC_TRANSFORM_EXECUTABLE})")
