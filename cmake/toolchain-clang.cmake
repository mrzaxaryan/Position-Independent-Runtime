# Clang toolchain file for bare-metal/PIC compilation
# Uses Generic system to avoid Windows/MSVC-specific CMake behavior

# Use Generic system to prevent Windows-specific linker configuration
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Use environment variables if set, otherwise default to clang/clang++
if(DEFINED ENV{CC})
    set(CMAKE_C_COMPILER $ENV{CC})
else()
    set(CMAKE_C_COMPILER clang)
endif()

if(DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++)
endif()

# Force compilers to be found without testing
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# Disable standard library linking
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_C_STANDARD_LIBRARIES "")

# Use GNU-style LLD linker
set(CMAKE_LINKER lld)
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> -fuse-ld=lld <LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> -fuse-ld=lld <LINK_FLAGS> <OBJECTS> -o <TARGET>")
