# alkos.libc.interface
#
# Interface library for libc. It provides the necessary include directories and
# a list of source files required to build a concrete version of the library.
#
# Usage:
#   1. Link against this target to inherit include directories:
#      target_link_libraries(your_target PRIVATE alkos.libc.interface)
#
#   2. Use the ALKOS_LIBC_SOURCES variable to create your own static library:
#      add_library(your_libc STATIC ${ALKOS_LIBC_SOURCES})

message(STATUS "Configuring LibC interface...")

# TODO: This currently works as interface to attach crti and crtn object files
# And should probably be kept that way and renamed
add_library(alkos.libc.config INTERFACE)

set(exclude_patterns ".*/(arch/.*|crt0\\.cpp)$")
alkos_find_sources(ALKOS_LIBC_SOURCES EXCLUDE ${exclude_patterns})
set(ALKOS_LIBC_SOURCES ${ALKOS_LIBC_SOURCES} CACHE INTERNAL "Source files for libc")

add_library(alkos.libc.interface INTERFACE)

target_include_directories(alkos.libc.interface INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/abi"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/internal"

    # At the time of their making, both libc and libcpp were in the same directory
    # Decoupling them now brings too little benefit and too much complexity to justify the effort
    "${CMAKE_CURRENT_SOURCE_DIR}/../libcpp/include"
)

target_link_libraries(alkos.libc.interface INTERFACE
    alkos.libcpp.interface
    alkos.autogen.interface
)

################################################################################
# CRT0 - C Runtime Startup Object Library
################################################################################
#
# The CRT0 (C Runtime 0) contains the _start entry point for userspace programs
# and is responsible for setting up the runtime environment before transferring
# control to the main() function.
#
add_library(alkos.libc.crt0 OBJECT
    src/internal/crt0.cpp
)
target_link_libraries(alkos.libc.crt0 PUBLIC alkos.libc.interface)

set(CRT0_OBJ_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.libc.crt0.dir/src/internal/crt0.cpp.o"
)
set_property(TARGET alkos.libc.config PROPERTY CRT0_OBJ "${CRT0_OBJ_PATH}")

####################### Architecture-Specific Code ########################

add_subdirectory(arch/${ARCH})
