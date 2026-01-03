message(STATUS "    -> gui_test2")
alkos_find_sources(GUI_TEST2_SOURCES)
alkos_register_userspace_app(gui_test2 "${GUI_TEST2_SOURCES}")

add_library(gui_test2.libgraphics STATIC ${LIBGRAPHICS_SOURCES})
target_link_libraries(gui_test2.libgraphics PUBLIC
    userspace.libgraphics.interface
    alkos.libc.interface
)

target_link_libraries(gui_test2 PRIVATE
    gui_test2.libgraphics
)
