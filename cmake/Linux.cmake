# =============================================================================
# Linux.cmake - Linux Platform Configuration
# =============================================================================

include_guard(GLOBAL)

# Target triples
set(_triples_i386    "i386-unknown-linux-gnu")
set(_triples_x86_64  "x86_64-unknown-linux-gnu")
set(_triples_armv7a  "armv7a-unknown-linux-gnueabihf")
set(_triples_aarch64 "aarch64-unknown-linux-gnu")
set(CPPPIC_TRIPLE "${_triples_${CPPPIC_ARCH}}")
set(CPPPIC_EXT ".elf")

# Platform include path
list(APPEND CPPPIC_INCLUDE_PATHS ${CMAKE_SOURCE_DIR}/include/pal/linux/)

# Filter sources
list(FILTER CPPPIC_SOURCES EXCLUDE REGEX ".*/windows/.*")
list(FILTER CPPPIC_SOURCES EXCLUDE REGEX ".*/uefi/.*")

# Compiler flags
list(APPEND CPPPIC_BASE_FLAGS -target ${CPPPIC_TRIPLE})

# Linker flags
set(_link "-Wl,-e,_start,-T,${CMAKE_SOURCE_DIR}/linker_script.txt,--build-id=none")
if(CPPPIC_BUILD_TYPE STREQUAL "debug")
    string(APPEND _link ",-Map,${CPPPIC_OUTPUT_DIR}/output.map.txt")
else()
    string(APPEND _link ",--strip-all,--gc-sections,-Map,${CPPPIC_OUTPUT_DIR}/output.map.txt")
endif()
list(APPEND CPPPIC_BASE_LINK_FLAGS -target ${CPPPIC_TRIPLE} "SHELL:${_link}")
