# =============================================================================
# RemovePhdr.cmake - Remove PT_PHDR from ELF (run via cmake -P)
# =============================================================================
# Usage: cmake -DELF_FILE=<path> -P RemovePhdr.cmake
#
# Solaris kernel fix: the Solaris/illumos kernel rejects static executables
# that contain a PT_PHDR program header without a PT_INTERP header:
#
#   /* in elfexec(), after mapelfexec(): */
#   if (uphdr != NULL && intphdr == NULL)
#       goto bad;
#
# LLD always emits PT_PHDR and provides no flag to suppress it.  Since this
# project produces fully static, freestanding binaries (no dynamic linker),
# PT_PHDR is semantically meaningless.  This script patches p_type from
# PT_PHDR (6) to PT_NULL (0) so the Solaris kernel ignores the entry.
#
# For 64-bit ELF (ELFCLASS64):
#   e_phoff     = bytes 32..39  (8 bytes, little-endian)
#   e_phentsize = bytes 54..55  (2 bytes, little-endian)
#   e_phnum     = bytes 56..57  (2 bytes, little-endian)
#   Each program header: p_type at offset 0 (4 bytes, little-endian)
#   PT_PHDR = 6
#
# For 32-bit ELF (ELFCLASS32):
#   e_phoff     = bytes 28..31  (4 bytes, little-endian)
#   e_phentsize = bytes 42..43  (2 bytes, little-endian)
#   e_phnum     = bytes 44..45  (2 bytes, little-endian)
#   Each program header: p_type at offset 0 (4 bytes, little-endian)
# =============================================================================

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED ELF_FILE)
    message(FATAL_ERROR "ELF_FILE is required")
endif()
if(NOT EXISTS "${ELF_FILE}")
    message(FATAL_ERROR "ELF file not found: ${ELF_FILE}")
endif()

# Read the ELF binary into a hex string
file(READ "${ELF_FILE}" _elf_hex HEX)

# Determine ELF class (byte 4): 1 = 32-bit, 2 = 64-bit
string(SUBSTRING "${_elf_hex}" 8 2 _ei_class_hex)
if(_ei_class_hex STREQUAL "01")
    # ELFCLASS32
    set(_phoff_offset 56)   # byte 28 * 2 hex chars
    set(_phoff_size   8)    # 4 bytes * 2
    set(_phent_offset 84)   # byte 42 * 2
    set(_phnum_offset 88)   # byte 44 * 2
elseif(_ei_class_hex STREQUAL "02")
    # ELFCLASS64
    set(_phoff_offset 64)   # byte 32 * 2
    set(_phoff_size   16)   # 8 bytes * 2
    set(_phent_offset 108)  # byte 54 * 2
    set(_phnum_offset 112)  # byte 56 * 2
else()
    message(FATAL_ERROR "Unknown ELF class: 0x${_ei_class_hex}")
endif()

# Helper: convert little-endian hex to integer
macro(le_hex_to_int hex_str out_var)
    set(_hex "${${hex_str}}")
    string(LENGTH "${_hex}" _len)
    set(_val 0)
    set(_shift 0)
    set(_pos 0)
    while(_pos LESS _len)
        string(SUBSTRING "${_hex}" ${_pos} 2 _byte)
        # Convert hex byte to decimal
        math(EXPR _byte_val "0x${_byte}")
        math(EXPR _val "${_val} + (${_byte_val} << ${_shift})")
        math(EXPR _shift "${_shift} + 8")
        math(EXPR _pos "${_pos} + 2")
    endwhile()
    set(${out_var} ${_val})
endmacro()

# Read e_phoff (program header table offset)
string(SUBSTRING "${_elf_hex}" ${_phoff_offset} ${_phoff_size} _phoff_hex)
le_hex_to_int(_phoff_hex _e_phoff)

# Read e_phentsize (program header entry size)
string(SUBSTRING "${_elf_hex}" ${_phent_offset} 4 _phent_hex)
le_hex_to_int(_phent_hex _e_phentsize)

# Read e_phnum (number of program headers)
string(SUBSTRING "${_elf_hex}" ${_phnum_offset} 4 _phnum_hex)
le_hex_to_int(_phnum_hex _e_phnum)

# PT_PHDR = 6, little-endian = 06 00 00 00
set(_pt_phdr_le "06000000")
set(_pt_null_le "00000000")

set(_found FALSE)
set(_i 0)
while(_i LESS _e_phnum)
    # Calculate byte offset of this program header's p_type
    math(EXPR _phdr_byte_off "${_e_phoff} + ${_i} * ${_e_phentsize}")
    math(EXPR _phdr_hex_off "${_phdr_byte_off} * 2")

    # Read p_type (4 bytes = 8 hex chars)
    string(SUBSTRING "${_elf_hex}" ${_phdr_hex_off} 8 _ptype_hex)

    if(_ptype_hex STREQUAL "${_pt_phdr_le}")
        set(_found TRUE)
        # Patch p_type from PT_PHDR to PT_NULL
        if(CMAKE_HOST_WIN32)
            execute_process(
                COMMAND powershell -NoProfile -Command
                    "$f=[System.IO.File]::OpenWrite('${ELF_FILE}'); $f.Seek(${_phdr_byte_off},0)|Out-Null; $b=[byte[]]@(0,0,0,0); $f.Write($b,0,4); $f.Close()"
                RESULT_VARIABLE _result
            )
        else()
            execute_process(
                COMMAND sh -c "printf '\\0\\0\\0\\0' | dd of='${ELF_FILE}' bs=1 seek=${_phdr_byte_off} count=4 conv=notrunc 2>/dev/null"
                RESULT_VARIABLE _result
            )
        endif()

        if(NOT _result EQUAL 0)
            message(FATAL_ERROR "Failed to patch PT_PHDR in: ${ELF_FILE}")
        endif()

        message(STATUS "Patched PT_PHDR -> PT_NULL at program header ${_i} in: ${ELF_FILE}")
        break()
    endif()

    math(EXPR _i "${_i} + 1")
endwhile()

if(NOT _found)
    message(STATUS "No PT_PHDR found in: ${ELF_FILE} (OK)")
endif()
