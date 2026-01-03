message(STATUS "    -> shell")
alkos_find_sources(SHELL_SOURCES)
alkos_register_userspace_app(shell "${SHELL_SOURCES}")

add_library(shell.libgraphics STATIC ${LIBGRAPHICS_SOURCES})
target_link_libraries(shell.libgraphics PUBLIC
    alkos.libgraphics.interface
    alkos.libc.interface
)

target_link_libraries(shell PUBLIC
    alkos.libcontainers.interface
    alkos.libcpp.interface
    shell.libgraphics
)
