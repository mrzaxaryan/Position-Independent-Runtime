# =============================================================================
# PICTransform.cmake - Acquire the pic-transform LLVM pass tool
# =============================================================================
# Acquires pic-transform (in order of preference):
#   1. Pre-installed on PATH
#   2. Built from in-tree source (tools/pic-transform)
#
# The pass eliminates .rodata/.rdata/.data/.bss sections by transforming global
# constants into stack-local immediate values automatically during compilation.
#
# Outputs (at most one of these will be set):
#   PIC_TRANSFORM_PLUGIN      - Path to loadable pass plugin (.so/.dylib/.dll)
#   PIC_TRANSFORM_EXECUTABLE  - Path to standalone pic-transform binary
#   PIC_TRANSFORM_TARGET      - CMake target to add as build dependency

include_guard(GLOBAL)

# =============================================================================
# Host platform detection
# =============================================================================
set(_host_os    "${CMAKE_HOST_SYSTEM_NAME}")
set(_host_arch  "${CMAKE_HOST_SYSTEM_PROCESSOR}")

# Normalise architecture names
string(TOLOWER "${_host_arch}" _host_arch_lower)
if(_host_arch_lower MATCHES "^(aarch64|arm64)$")
    set(_arch_tag "aarch64")
elseif(_host_arch_lower MATCHES "^(x86_64|amd64)$")
    set(_arch_tag "x86_64")
else()
    set(_arch_tag "${_host_arch_lower}")
endif()

# Platform-specific naming
if(_host_os STREQUAL "Linux")
    set(_platform_tag   "linux-${_arch_tag}")
    set(_archive_ext    ".tar.gz")
    set(_bin_name       "pic-transform")
    set(_plugin_name    "PICTransform.so")
elseif(_host_os STREQUAL "Darwin")
    if(_arch_tag STREQUAL "aarch64")
        set(_platform_tag "macos-arm64")
    else()
        set(_platform_tag "macos-${_arch_tag}")
    endif()
    set(_archive_ext    ".tar.gz")
    set(_bin_name       "pic-transform")
    set(_plugin_name    "PICTransform.dylib")
elseif(_host_os STREQUAL "Windows")
    set(_platform_tag   "windows-${_arch_tag}")
    set(_archive_ext    ".zip")
    set(_bin_name       "pic-transform.exe")
    set(_plugin_name    "PICTransform.dll")
else()
    message(FATAL_ERROR "pic-transform: unsupported host: ${_host_os} ${_host_arch}")
endif()

# -----------------------------------------------------------------------------
# Helper: set output variables and return
# -----------------------------------------------------------------------------
macro(_pt_found mode path)
    if("${mode}" STREQUAL "plugin")
        set(PIC_TRANSFORM_PLUGIN     "${path}")
        set(PIC_TRANSFORM_EXECUTABLE "")
    else()
        set(PIC_TRANSFORM_PLUGIN     "")
        set(PIC_TRANSFORM_EXECUTABLE "${path}")
    endif()
    set(PIC_TRANSFORM_TARGET "")
    message(STATUS "pic-transform: ${ARGN} (${path})")
    return()
endmacro()

# Helper: find a built artifact in both single-config and multi-config layouts
macro(_pt_find_artifact var name build_dir)
    set(${var} "")
    if(EXISTS "${build_dir}/${name}")
        set(${var} "${build_dir}/${name}")
    elseif(EXISTS "${build_dir}/Release/${name}")
        set(${var} "${build_dir}/Release/${name}")
    endif()
endmacro()

# =============================================================================
# Strategy 1: Pre-installed on PATH
# =============================================================================
find_program(_pt_system_bin pic-transform)

if(_pt_system_bin)
    _pt_found(standalone "${_pt_system_bin}" "using system binary")
endif()

# =============================================================================
# Strategy 2: Build from in-tree source
# =============================================================================
# Builds tools/pic-transform if LLVM dev files are available.
# Override LLVM location: -DPIC_TRANSFORM_LLVM_DIR=/path/to/lib/cmake/llvm

set(_pt_source_dir "${PIR_ROOT_DIR}/tools/pic-transform")

if(EXISTS "${_pt_source_dir}/CMakeLists.txt")
    # ── Locate LLVM cmake config from the compiler installation ─────────
    if(NOT DEFINED PIC_TRANSFORM_LLVM_DIR)
        find_program(_pt_clang "${CMAKE_CXX_COMPILER}")
        if(_pt_clang)
            get_filename_component(_pt_clang_real "${_pt_clang}" REALPATH)
            get_filename_component(_pt_llvm_root "${_pt_clang_real}/../../" ABSOLUTE)
            foreach(_dir "${_pt_llvm_root}/lib/cmake/llvm"
                         "${_pt_llvm_root}/lib64/cmake/llvm")
                if(EXISTS "${_dir}/LLVMConfig.cmake")
                    set(PIC_TRANSFORM_LLVM_DIR "${_dir}")
                    break()
                endif()
            endforeach()
        endif()
    endif()

    if(DEFINED PIC_TRANSFORM_LLVM_DIR AND EXISTS "${PIC_TRANSFORM_LLVM_DIR}/LLVMConfig.cmake")
        set(_pt_build_dir "${CMAKE_BINARY_DIR}/pic-transform-build")

        # Configure (skip if already configured)
        if(NOT EXISTS "${_pt_build_dir}/CMakeCache.txt")
            message(STATUS "pic-transform: configuring (LLVM_DIR=${PIC_TRANSFORM_LLVM_DIR})")
            # Use the LLVM clang that matches the LLVM dev libraries
            get_filename_component(_pt_compiler_dir "${_pt_clang_real}" DIRECTORY)
            set(_pt_host_cc  "${_pt_compiler_dir}/clang")
            set(_pt_host_cxx "${_pt_compiler_dir}/clang++")
            execute_process(
                COMMAND ${CMAKE_COMMAND}
                    -S "${_pt_source_dir}"
                    -B "${_pt_build_dir}"
                    -DLLVM_DIR=${PIC_TRANSFORM_LLVM_DIR}
                    -DCMAKE_BUILD_TYPE=Release
                    -DCMAKE_C_COMPILER=${_pt_host_cc}
                    -DCMAKE_CXX_COMPILER=${_pt_host_cxx}
                    -DSTATIC_LINK=ON
                RESULT_VARIABLE _pt_cfg_rc
                OUTPUT_VARIABLE _pt_cfg_out
                ERROR_VARIABLE  _pt_cfg_err
            )
        else()
            set(_pt_cfg_rc 0)
        endif()

        if(_pt_cfg_rc EQUAL 0)
            message(STATUS "pic-transform: building from source...")
            execute_process(
                COMMAND ${CMAKE_COMMAND} --build "${_pt_build_dir}" --config Release
                RESULT_VARIABLE _pt_build_rc
                OUTPUT_VARIABLE _pt_build_out
                ERROR_VARIABLE  _pt_build_err
            )

            if(_pt_build_rc EQUAL 0)
                # Prefer plugin over standalone
                _pt_find_artifact(_pt_plugin "${_plugin_name}" "${_pt_build_dir}")
                if(_pt_plugin)
                    _pt_found(plugin "${_pt_plugin}" "built plugin from source")
                endif()

                _pt_find_artifact(_pt_bin "${_bin_name}" "${_pt_build_dir}")
                if(_pt_bin)
                    _pt_found(standalone "${_pt_bin}" "built standalone from source")
                endif()
            endif()
        endif()

        # Source build failed
        if(NOT _pt_cfg_rc EQUAL 0)
            message(FATAL_ERROR "pic-transform: configure failed:\n${_pt_cfg_err}")
        elseif(NOT _pt_build_rc EQUAL 0)
            message(FATAL_ERROR "pic-transform: build failed:\n${_pt_build_err}")
        else()
            message(FATAL_ERROR "pic-transform: build succeeded but no artifact found")
        endif()
    else()
        message(FATAL_ERROR "pic-transform: LLVM dev files not found (need LLVMConfig.cmake)")
    endif()
else()
    message(FATAL_ERROR "pic-transform: source not found at ${_pt_source_dir}")
endif()
