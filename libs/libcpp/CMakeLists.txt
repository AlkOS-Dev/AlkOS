# alkos.libcpp.interface
#
# Interface library for libcpp. It provides the necessary include directories and
# a list of source files required to build a concrete version of the library.
#
# Usage:
#   1. Link against this target to inherit include directories:
#      target_link_libraries(your_target PRIVATE alkos.libcpp.interface)
#
#   2. Use the ALKOS_LIBCPP_SOURCES variable to create your own static library:
#      add_library(your_libcpp STATIC ${ALKOS_LIBCPP_SOURCES})

message(STATUS "Configuring LibCpp interface...")

alkos_find_sources(ALKOS_LIBCPP_SOURCES)
set(ALKOS_LIBCPP_SOURCES ${ALKOS_LIBCPP_SOURCES} CACHE INTERNAL "Source files for libcpp")

add_library(alkos.libcpp.interface INTERFACE)

target_include_directories(alkos.libcpp.interface INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"

    # At the time of their making, both libc and libcpp were in the same directory
    # Decoupling them now brings too little benefit and too much complexity to justify the effort
    "${CMAKE_CURRENT_SOURCE_DIR}/../libc/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../libc/abi"
)

target_link_libraries(alkos.libcpp.interface INTERFACE
    alkos.libtemplate.interface
)
