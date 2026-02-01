# =============================================================================
# Windows.cmake - Windows Platform Configuration
# =============================================================================

include_guard(GLOBAL)

# Target triples
set(_triples_i386    "i386-pc-windows-gnu")
set(_triples_x86_64  "x86_64-pc-windows-gnu")
set(_triples_armv7a  "armv7a-pc-windows-gnu")
set(_triples_aarch64 "aarch64-pc-windows-gnu")
set(CPPPIC_TRIPLE "${_triples_${CPPPIC_ARCH}}")
set(CPPPIC_EXT ".exe")

# Platform include path
list(APPEND CPPPIC_INCLUDE_PATHS ${CMAKE_SOURCE_DIR}/include/pal/windows/)

# Filter sources
list(FILTER CPPPIC_SOURCES EXCLUDE REGEX ".*/linux/.*")
list(FILTER CPPPIC_SOURCES EXCLUDE REGEX ".*/uefi/.*")

# Compiler flags
list(APPEND CPPPIC_BASE_FLAGS -target ${CPPPIC_TRIPLE})
if(CPPPIC_BUILD_TYPE STREQUAL "debug")
    list(APPEND CPPPIC_BASE_FLAGS -gcodeview)
endif()
if(CPPPIC_ARCH MATCHES "^(armv7a|aarch64)$")
    list(APPEND CPPPIC_BASE_FLAGS -mstack-probe-size=0)
endif()

# Linker flags
set(_link "-Wl,/Entry:_start,/SUBSYSTEM:CONSOLE,/ORDER:@${CMAKE_SOURCE_DIR}/orderfile.txt,/MERGE:.rdata=.text")
if(CPPPIC_ARCH STREQUAL "i386")
    string(APPEND _link ",/BASE:0x400000,/FILEALIGN:0x1000,/SAFESEH:NO")
endif()
if(CPPPIC_BUILD_TYPE STREQUAL "debug")
    string(APPEND _link ",/DEBUG,/MAP:${CPPPIC_OUTPUT_DIR}/output.map.txt")
else()
    string(APPEND _link ",--strip-all,/OPT:REF,/OPT:ICF,/RELEASE,/LTCG,/MAP:${CPPPIC_OUTPUT_DIR}/output.map.txt")
endif()
list(APPEND CPPPIC_BASE_LINK_FLAGS -target ${CPPPIC_TRIPLE} "SHELL:${_link}")
