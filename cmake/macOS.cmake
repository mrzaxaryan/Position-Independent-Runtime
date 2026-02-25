# =============================================================================
# macOS.cmake - macOS Platform Configuration
# =============================================================================

include_guard(GLOBAL)

# Validate: macOS only supports x86_64 and aarch64
if(NOT CPPPIC_ARCH MATCHES "^(x86_64|aarch64)$")
    message(FATAL_ERROR "macOS only supports x86_64 and aarch64 (got: ${CPPPIC_ARCH})")
endif()

cpppic_get_target_info()
cpppic_filter_sources(windows linux uefi)

list(APPEND CPPPIC_INCLUDE_PATHS "${CMAKE_SOURCE_DIR}/include/platform/macos")
list(APPEND CPPPIC_BASE_FLAGS -target ${CPPPIC_TRIPLE})

# macOS-specific compiler flags
list(APPEND CPPPIC_BASE_FLAGS -fno-stack-protector)

if(CPPPIC_ARCH STREQUAL "x86_64")
    # Disable the red zone for x86_64. The System V ABI allows leaf functions to
    # use 128 bytes below RSP without adjusting RSP (the "red zone"). However,
    # PIR executes syscalls via inline asm, and under Rosetta 2 (Apple Silicon
    # emulating x86_64) the syscall translation can clobber the red zone. At -O1+
    # with LTO, the optimizer may inline System::Call into what becomes a leaf
    # function, placing syscall buffers in the red zone — leading to SIGSEGV when
    # Rosetta 2 overwrites them. This flag is already used for UEFI x86_64
    # (where interrupts have the same effect) and is standard practice for
    # position-independent code that makes direct syscalls.
    list(APPEND CPPPIC_BASE_FLAGS -mno-red-zone)
elseif(CPPPIC_ARCH STREQUAL "aarch64")
    list(APPEND CPPPIC_BASE_FLAGS -mstack-probe-size=0)
endif()

# Linker configuration (ld64.lld / Mach-O)
cpppic_add_link_flags(
    -e,_entry_point
    -static
    -platform_version,macos,11.0.0,11.0.0
    -order_file,${CMAKE_SOURCE_DIR}/cmake/function.order.macos
    -map,${CPPPIC_MAP_FILE}
)

if(CPPPIC_BUILD_TYPE STREQUAL "release")
    cpppic_add_link_flags(
        -dead_strip
    )
endif()

list(APPEND CPPPIC_BASE_LINK_FLAGS -target ${CPPPIC_TRIPLE})
