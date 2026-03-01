# =============================================================================
# Solaris.cmake - Solaris/illumos Platform Configuration
# =============================================================================

include_guard(GLOBAL)

# Validate: Solaris only supports x86_64
if(NOT PIR_ARCH STREQUAL "x86_64")
    message(FATAL_ERROR "Solaris only supports x86_64 (got: ${PIR_ARCH})")
endif()

pir_get_target_info()
pir_filter_sources(windows linux macos uefi)

list(APPEND PIR_INCLUDE_PATHS
    "${CMAKE_SOURCE_DIR}/src/platform/os/solaris"
    "${CMAKE_SOURCE_DIR}/src/platform/os/solaris/common")

# Disable the red zone for x86_64 (same rationale as Linux/macOS/UEFI)
list(APPEND PIR_BASE_FLAGS -mno-red-zone)

# Clang's Solaris target defaults to -fPIC and medium code model, producing
# R_X86_64_32 relocations that fail at link time. Force small code model
# (matching Linux behavior) for our freestanding binary.
list(APPEND PIR_BASE_FLAGS -fno-pic -mcmodel=small)

# Linker configuration (ELF via LLD)
pir_add_link_flags(
    -e,entry_point
    --no-dynamic-linker
    --no-pie
    --symbol-ordering-file=${CMAKE_SOURCE_DIR}/cmake/data/function.order.solaris
    --build-id=none
    -Map,${PIR_MAP_FILE}
)

if(PIR_BUILD_TYPE STREQUAL "release")
    pir_add_link_flags(--strip-all --gc-sections)
endif()

# Clang's Solaris driver has no -fuse-ld=lld support; use Linux triple for
# the link step since the output is freestanding ELF (same format as Linux).
set(PIR_LINK_TRIPLE "x86_64-unknown-linux-gnu")
list(APPEND PIR_BASE_LINK_FLAGS -fuse-ld=lld)
