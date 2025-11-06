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

alkos_find_sources(ALKOS_LIBC_SOURCES)
set(ALKOS_LIBC_SOURCES ${ALKOS_LIBC_SOURCES} CACHE INTERNAL "Source files for libc")

add_library(alkos.libc.interface INTERFACE)

target_include_directories(alkos.libc.interface INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/abi"

    # At the time of their making, both libc and libcpp were in the same directory
    # Decoupling them now brings too little benefit and too much complexity to justify the effort
    "${CMAKE_CURRENT_SOURCE_DIR}/../libcpp/include"
)

target_include_directories(alkos.libc.interface INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/internal"
)
