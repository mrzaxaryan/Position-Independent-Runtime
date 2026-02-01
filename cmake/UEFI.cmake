# =============================================================================
# UEFI.cmake - UEFI Platform Configuration
# =============================================================================

include_guard(GLOBAL)

# UEFI only supports x86_64 and aarch64
if(NOT CPPPIC_ARCH MATCHES "^(x86_64|aarch64)$")
    message(FATAL_ERROR "UEFI only supports x86_64 and aarch64")
endif()

# Target triples (PE/COFF format via windows-gnu)
set(_triples_x86_64  "x86_64-pc-windows-gnu")
set(_triples_aarch64 "aarch64-pc-windows-gnu")
set(CPPPIC_TRIPLE "${_triples_${CPPPIC_ARCH}}")
set(CPPPIC_EXT ".efi")

# Platform include path
list(APPEND CPPPIC_INCLUDE_PATHS ${CMAKE_SOURCE_DIR}/include/pal/uefi/)

# Filter sources
list(FILTER CPPPIC_SOURCES EXCLUDE REGEX ".*/windows/.*")
list(FILTER CPPPIC_SOURCES EXCLUDE REGEX ".*/linux/.*")

# Compiler flags
list(APPEND CPPPIC_BASE_FLAGS -target ${CPPPIC_TRIPLE})
if(CPPPIC_ARCH STREQUAL "x86_64")
    list(APPEND CPPPIC_BASE_FLAGS -mno-red-zone)
endif()
if(CPPPIC_ARCH STREQUAL "aarch64")
    list(APPEND CPPPIC_BASE_FLAGS -mstack-probe-size=0)
endif()

# Linker flags (no .rdata merge for UEFI)
set(_link "-Wl,/Entry:efi_main,/SUBSYSTEM:EFI_APPLICATION,/NODEFAULTLIB")
if(CPPPIC_BUILD_TYPE STREQUAL "debug")
    string(APPEND _link ",/DEBUG,/MAP:${CPPPIC_OUTPUT_DIR}/output.map.txt")
else()
    string(APPEND _link ",/OPT:REF,/OPT:ICF,/MAP:${CPPPIC_OUTPUT_DIR}/output.map.txt")
endif()
list(APPEND CPPPIC_BASE_LINK_FLAGS -target ${CPPPIC_TRIPLE} "SHELL:${_link}")

# UEFI boot directory setup
function(cpppic_add_uefi_boot target_name)
    set(_boot_dir "${CPPPIC_OUTPUT_DIR}/EFI/BOOT")
    if(CPPPIC_ARCH STREQUAL "x86_64")
        set(_boot_file "BOOTX64.EFI")
    else()
        set(_boot_file "BOOTAA64.EFI")
    endif()
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_boot_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CPPPIC_OUTPUT_DIR}/output${CPPPIC_EXT}" "${_boot_dir}/${_boot_file}"
        COMMENT "Creating UEFI boot structure..."
        VERBATIM
    )
endfunction()
