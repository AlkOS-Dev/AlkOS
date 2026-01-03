message(STATUS "    -> gui_test")
alkos_find_sources(GUI_TEST_SOURCES)
alkos_register_userspace_app(gui_test "${GUI_TEST_SOURCES}")

add_library(gui_test.libgraphics STATIC ${LIBGRAPHICS_SOURCES})
target_link_libraries(gui_test.libgraphics PUBLIC
    userspace.libgraphics.interface
    alkos.libc.interface
)

target_link_libraries(gui_test PRIVATE
    gui_test.libgraphics
)
