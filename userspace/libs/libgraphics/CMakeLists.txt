message(STATUS "    -> libgraphics")

alkos_find_sources(LIBGRAPHICS_SOURCES)
set(LIBGRAPHICS_SOURCES ${LIBGRAPHICS_SOURCES} CACHE INTERNAL "Source files for libgraphics")

add_library(userspace.libgraphics.interface INTERFACE)

target_include_directories(userspace.libgraphics.interface INTERFACE
    "include/"
)

target_link_libraries(userspace.libgraphics.interface INTERFACE
    alkos.libcpp.interface
)
