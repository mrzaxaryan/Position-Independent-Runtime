# =============================================================================
# cpp-pic - Position-Independent Code Build System
# =============================================================================

cmake_minimum_required(VERSION 3.20)

# Toolchain (before project)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER "$ENV{CC}" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER "$ENV{CXX}" CACHE STRING "" FORCE)
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER clang)
endif()
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif()
foreach(lang C CXX)
    set(CMAKE_${lang}_COMPILER_FORCED TRUE)
    set(CMAKE_${lang}_COMPILER_WORKS TRUE)
    set(CMAKE_${lang}_STANDARD_LIBRARIES "")
endforeach()
set(CMAKE_LINKER lld)
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> -fuse-ld=lld <LINK_FLAGS> <OBJECTS> -o <TARGET>")

project(cpp-pic LANGUAGES CXX)

# Load common config
include(${CMAKE_SOURCE_DIR}/cmake/Common.cmake)

# Load platform-specific config
if(CPPPIC_PLATFORM STREQUAL "windows")
    include(${CMAKE_SOURCE_DIR}/cmake/Windows.cmake)
elseif(CPPPIC_PLATFORM STREQUAL "linux")
    include(${CMAKE_SOURCE_DIR}/cmake/Linux.cmake)
elseif(CPPPIC_PLATFORM STREQUAL "uefi")
    include(${CMAKE_SOURCE_DIR}/cmake/UEFI.cmake)
endif()

# Create output directory
file(MAKE_DIRECTORY "${CPPPIC_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CPPPIC_OUTPUT_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CPPPIC_OUTPUT_DIR}")

# Create target
add_executable(${CPPPIC_TRIPLE} ${CPPPIC_SOURCES} ${CPPPIC_HEADERS})
set_target_properties(${CPPPIC_TRIPLE} PROPERTIES OUTPUT_NAME "output" SUFFIX "${CPPPIC_EXT}")
target_include_directories(${CPPPIC_TRIPLE} PRIVATE ${CPPPIC_INCLUDE_PATHS})
target_compile_definitions(${CPPPIC_TRIPLE} PRIVATE ${CPPPIC_DEFINES})
target_compile_options(${CPPPIC_TRIPLE} PRIVATE ${CPPPIC_BASE_FLAGS})
target_link_options(${CPPPIC_TRIPLE} PRIVATE ${CPPPIC_BASE_LINK_FLAGS})

# Post-build
cpppic_add_postbuild(${CPPPIC_TRIPLE})
if(CPPPIC_PLATFORM STREQUAL "uefi")
    cpppic_add_uefi_boot(${CPPPIC_TRIPLE})
endif()

# Summary
message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "BUILD_TYPE:    ${CPPPIC_BUILD_TYPE}")
message(STATUS "PLATFORM:      ${CPPPIC_PLATFORM}")
message(STATUS "ARCHITECTURE:  ${CPPPIC_ARCH}")
message(STATUS "OPTIMIZATION:  -${CPPPIC_OPT_LEVEL}")
message(STATUS "TARGET_TRIPLE: ${CPPPIC_TRIPLE}")
message(STATUS "OUTPUT:        ${CPPPIC_OUTPUT_DIR}/output${CPPPIC_EXT}")
message(STATUS "===========================")
message(STATUS "")
