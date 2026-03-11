# =============================================================================
# PICTransform.cmake - Build and use the pic-transform LLVM pass
# =============================================================================
# Builds pic-transform as a host tool (separate from the freestanding PIR build)
# and provides a function to run it on bitcode files.
#
# The pass eliminates .rodata/.rdata/.data/.bss sections by transforming global
# constants into stack-local immediate values -- the same transformation that
# PIR's _embed infrastructure does, but applied automatically by the compiler.
#
# Usage in the build pipeline:
#   1. Compile .cpp -> .bc (emit LLVM bitcode instead of object files)
#   2. Run pic-transform on each .bc file
#   3. Compile transformed .bc -> .o (normal object files)
#   4. Link .o files into final binary

include_guard(GLOBAL)

set(PIC_TRANSFORM_DIR "${PIR_ROOT_DIR}/pic-transform")

# Skip if submodule not checked out
if(NOT EXISTS "${PIC_TRANSFORM_DIR}/CMakeLists.txt")
    message(STATUS "pic-transform submodule not found -- skipping")
    return()
endif()

# =============================================================================
# Option to enable/disable pic-transform
# =============================================================================
option(PIR_USE_PIC_TRANSFORM "Use pic-transform pass to auto-eliminate data sections" OFF)

if(NOT PIR_USE_PIC_TRANSFORM)
    return()
endif()

# =============================================================================
# Find or build pic-transform
# =============================================================================
# First check if pic-transform is already available on PATH
find_program(PIC_TRANSFORM_EXECUTABLE pic-transform)

if(NOT PIC_TRANSFORM_EXECUTABLE)
    # Build it as an external project (host tool, not cross-compiled)
    include(ExternalProject)

    # Find LLVM for the host build
    find_program(_HOST_LLVM_CONFIG llvm-config)
    if(_HOST_LLVM_CONFIG)
        execute_process(
            COMMAND ${_HOST_LLVM_CONFIG} --cmakedir
            OUTPUT_VARIABLE _HOST_LLVM_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()

    if(NOT _HOST_LLVM_DIR)
        # Try common locations
        foreach(_ver 22 21 20)
            foreach(_prefix
                "/usr/lib/llvm-${_ver}/lib/cmake/llvm"
                "/opt/homebrew/opt/llvm/lib/cmake/llvm"
                "/usr/local/opt/llvm/lib/cmake/llvm"
            )
                if(EXISTS "${_prefix}/LLVMConfig.cmake")
                    set(_HOST_LLVM_DIR "${_prefix}")
                    break()
                endif()
            endforeach()
            if(_HOST_LLVM_DIR)
                break()
            endif()
        endforeach()
    endif()

    if(NOT _HOST_LLVM_DIR)
        message(WARNING "pic-transform: Cannot find LLVM development files for host build. "
                        "Install llvm-dev or set LLVM_DIR. Disabling pic-transform.")
        set(PIR_USE_PIC_TRANSFORM OFF)
        return()
    endif()

    set(_PIC_TRANSFORM_BUILD_DIR "${CMAKE_BINARY_DIR}/pic-transform-build")
    set(_PIC_TRANSFORM_BIN "${_PIC_TRANSFORM_BUILD_DIR}/pic-transform")

    # Determine host compiler (use the system compiler, not the cross-compiler)
    find_program(_HOST_CXX NAMES c++ g++ clang++)
    find_program(_HOST_CC NAMES cc gcc clang)

    ExternalProject_Add(pic-transform-build
        SOURCE_DIR "${PIC_TRANSFORM_DIR}"
        BINARY_DIR "${_PIC_TRANSFORM_BUILD_DIR}"
        CMAKE_ARGS
            -DLLVM_DIR=${_HOST_LLVM_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_CXX_COMPILER=${_HOST_CXX}
            -DCMAKE_C_COMPILER=${_HOST_CC}
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
        INSTALL_COMMAND ""
        BUILD_BYPRODUCTS "${_PIC_TRANSFORM_BIN}"
    )

    # The executable path (standalone tool or plugin .so)
    if(EXISTS "${_PIC_TRANSFORM_BIN}")
        set(PIC_TRANSFORM_EXECUTABLE "${_PIC_TRANSFORM_BIN}")
    else()
        # Will be available after build
        set(PIC_TRANSFORM_EXECUTABLE "${_PIC_TRANSFORM_BIN}")
    endif()

    set(PIC_TRANSFORM_TARGET pic-transform-build)
    message(STATUS "pic-transform: building from submodule (LLVM_DIR=${_HOST_LLVM_DIR})")
else()
    message(STATUS "pic-transform: using system binary (${PIC_TRANSFORM_EXECUTABLE})")
    set(PIC_TRANSFORM_TARGET "")
endif()

# Also check for the plugin .so (for -fpass-plugin= usage)
find_file(PIC_TRANSFORM_PLUGIN
    NAMES PICTransform.so PICTransform.dylib
    PATHS "${_PIC_TRANSFORM_BUILD_DIR}" "${PIC_TRANSFORM_DIR}/build"
    NO_DEFAULT_PATH
)

# =============================================================================
# Function: Transform a bitcode file with pic-transform
# =============================================================================
# pir_pic_transform(<input.bc> <output.bc>)
function(pir_pic_transform INPUT OUTPUT)
    add_custom_command(
        OUTPUT "${OUTPUT}"
        COMMAND "${PIC_TRANSFORM_EXECUTABLE}" "${INPUT}" -o "${OUTPUT}"
        DEPENDS "${INPUT}" ${PIC_TRANSFORM_TARGET}
        COMMENT "pic-transform: ${INPUT}"
        VERBATIM
    )
endfunction()
