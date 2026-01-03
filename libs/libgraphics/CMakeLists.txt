message(STATUS "Configuring Graphics Library...")

alkos_find_sources(LIBGRAPHICS_SOURCES)
set(LIBGRAPHICS_SOURCES ${LIBGRAPHICS_SOURCES} CACHE INTERNAL "Source files for libgraphics")

add_library(alkos.libgraphics.interface INTERFACE)

target_include_directories(alkos.libgraphics.interface INTERFACE
    "include"
)

target_link_libraries(alkos.libgraphics.interface INTERFACE
    alkos.libcpp.interface
)
