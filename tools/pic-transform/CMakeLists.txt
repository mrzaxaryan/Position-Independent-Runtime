cmake_minimum_required(VERSION 3.20)
project(PICTransform LANGUAGES C CXX)

# ── Static-link C++ runtime for portable binaries ────────────────────────────
option(STATIC_LINK "Statically link C++ runtime" OFF)
if(STATIC_LINK AND MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ── Find LLVM ──────────────────────────────────────────────────────────────────
# Point LLVM_DIR to the cmake config directory, e.g.:
#   cmake -B build -DLLVM_DIR=/usr/lib/llvm-22/lib/cmake/llvm
#   cmake -B build -DLLVM_DIR="D:/git/clang+llvm-22.x/lib/cmake/llvm"
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# ── Check if plugins are supported ────────────────────────────────────────────
# Also verify PassPlugin.h exists (some distro packages set LLVM_ENABLE_PLUGINS
# but don't ship the header).
if(LLVM_ENABLE_PLUGINS)
	find_file(_PASS_PLUGIN_H "llvm/Passes/PassPlugin.h" PATHS ${LLVM_INCLUDE_DIRS} NO_DEFAULT_PATH)
	if(NOT _PASS_PLUGIN_H)
		message(STATUS "PassPlugin.h not found — falling back to standalone tool")
		set(LLVM_ENABLE_PLUGINS OFF)
	endif()
endif()

if(NOT LLVM_ENABLE_PLUGINS)
	message(STATUS "Building standalone pic-transform tool")

	llvm_map_components_to_libnames(LLVM_LIBS
		core
		support
		irreader
		bitwriter
		passes
		transformutils
	)

	add_executable(pic-transform
		PICTransformOpt.cpp
		PICTransformPass.cpp
	)
	target_compile_features(pic-transform PRIVATE cxx_std_17)
	target_compile_definitions(pic-transform PRIVATE PIC_TRANSFORM_STANDALONE)
	target_link_libraries(pic-transform PRIVATE ${LLVM_LIBS})

	if(STATIC_LINK)
		if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			target_link_options(pic-transform PRIVATE -static-libgcc -static-libstdc++)
		elseif(APPLE)
			target_link_options(pic-transform PRIVATE -static-libstdc++)
		endif()
	endif()

else()
	message(STATUS "LLVM plugins supported — building loadable pass plugin")

	add_library(PICTransform MODULE
		PICTransformPass.cpp
	)
	target_compile_features(PICTransform PRIVATE cxx_std_17)

	if(WIN32)
		target_link_libraries(PICTransform PRIVATE LLVM-C)
	endif()

	set_target_properties(PICTransform PROPERTIES
		PREFIX ""
		OUTPUT_NAME "PICTransform"
	)
endif()
