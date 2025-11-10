# Set the system name and processor
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# This makes CMake test the compiler by building a static library
# (So it doesn't need to link an executable against C standard library)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Specify the cross-compilers
set(PREFIX x86_64-elf)

if (NOT DEFINED TOOL_BINARIES_DIR)
    message(FATAL_ERROR "Missing crucial TOOL_BINARIES_DIR, possibly configuration went wrong...")
endif ()
message(STATUS "Using tooldir: ${TOOL_BINARIES_DIR}")

# Define the path to your cross-compiler tools
set(CROSS_COMPILER_PATH "${TOOL_BINARIES_DIR}/${PREFIX}/bin")

set(CMAKE_C_COMPILER "${CROSS_COMPILER_PATH}/${PREFIX}-gcc" CACHE FILEPATH "C Compiler" FORCE)
set(CMAKE_CXX_COMPILER "${CROSS_COMPILER_PATH}/${PREFIX}-g++" CACHE FILEPATH "C++ Compiler" FORCE)
set(CMAKE_AR "${CROSS_COMPILER_PATH}/${PREFIX}-ar" CACHE FILEPATH "Archiver" FORCE)
set(CMAKE_RANLIB "${CROSS_COMPILER_PATH}/${PREFIX}-ranlib" CACHE FILEPATH "Ranlib" FORCE)
set(CMAKE_LINKER "${CROSS_COMPILER_PATH}/${PREFIX}-ld" CACHE FILEPATH "Linker" FORCE)
set(CMAKE_OBJCOPY "${CROSS_COMPILER_PATH}/${PREFIX}-objcopy" CACHE FILEPATH "Objcopy" FORCE)
set(CMAKE_GDB "${CROSS_COMPILER_PATH}/${PREFIX}-gdb" CACHE FILEPATH "GDB" FORCE)
set(CMAKE_ASM_NASM_COMPILER "nasm" CACHE FILEPATH "NASM Assembler" FORCE)

# Setup the cmake environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER) # Don't search for programs on the host system
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)  # Only search for libraries in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)  # Only search for headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)  # Only search for packages in the target directories

# Optional: Specify NASM if it's not in your PATH
# set(CMAKE_ASM_NASM_COMPILER "/path/to/your/nasm")

# TODO: try to use g++ compiler
set(LOADER_PREFIX i386-elf)
set(LOADER_TOOLS_DIR "${TOOL_BINARIES_DIR}/${LOADER_PREFIX}/bin")
set(CMAKE_CXX_COMPILER_32 "${LOADER_TOOLS_DIR}/${LOADER_PREFIX}-g++" CACHE FILEPATH "C++ Compiler (32-bit)" FORCE)

# Set arch
set(ARCH "x86_64")
